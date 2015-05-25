print("LoadDevScriptBase")

-------------------------------------------------------------------------
--[[
buffer 类
备注:
	所有方法的参数的index都是从1开始
	字节数组保存在buffer.arr
	数组长度保存在buffer.len
	数组偏移量保存在buffer.offset
	table buffer_baseclass 不保存数据，只保存方法
	数据均放在new_buffer返回的table下而不是在buffer_baseclass

方法:
function buffer:SetBuffer(arr,narr)
function buffer:SetOffset(offset)
function buffer:ScalcByteIndex(byte_index)
--获取Buffer方法
function buffer:GetByte(byte_index)
function buffer:GetBCD(byte_index)
function buffer:GetWord(byte_index,swap_byte)
function buffer:GetDWord(byte_index,swap_word,swap_byte)
function buffer:GetUShort=GetWord
function buffer:GetShort(byte_index,swap_byte)
function buffer:GetUInt=GetDWord
function buffer:GetInt(byte_index,swap_word,swap_byte)
function buffer:GetFloat(byte_index,swap_word,swap_byte)
function buffer:GetDouble(byte_index)
--设置Buffer方法
function buffer:SetByte(byte_index,byte)
function buffer:SetBCD(byte_index,byte)
function buffer:SetWord(byte_index,word,swap_byte)
function buffer:SetDWord(byte_index,dword,swap_word,swap_byte)
function buffer:SetFloat(byte_index,float,swap_word,swap_byte)
]]

buffer_baseclass = {
	--[[
	arr={},
	len=0,
	offset=0,
	]]
	----------------------------------------------------------------------------------------------
	SetBuffer=function(self,arr,narr)
		local ret=nil
		if arr==nil then arr={} end
		if type(arr)=='table' then 
			self.arr=arr
			if narr then self.len=narr 
			else self.len=#arr end
			self.offset=0
			ret=self
		end
		return ret
	end,
	SetOffset=function(self,offset)
		if type(offset)=='number' then self.offset=offset
		else print("\n SetOffset error : offset type "..type(offset)) end
	end,
	ScalcByteIndex=function(self,byte_index)
		if byte_index==nil then byte_index=0 end
		return self.offset+byte_index
	end,
	PrintBuffer=function(self)
		print("PrintBuffer: len="..tostring(self.len).." offset="..tostring(self.offset).." bytes="..tostring(self.len-self.offset))
		local str=''
		for i=1+self.offset,self.len do
			if self.arr[i] then
				str=str..string.format("%02x ",self.arr[i])
			else
				str=str.."00 "
			end
		end
		print(str)
	end,
	----------------------------------------------------------------------------------------------
	--获取Buffer方法
	GetByte=function(self,byte_index)
		if byte_index==nil then byte_index=0 end
		byte_index=self:ScalcByteIndex(byte_index)
		if byte_index>self.len or self.arr[byte_index]==nil then
			return 0
		end
		return self.arr[byte_index]&0xff
	end,
	GetBCD=function(self,byte_index)
		--if byte_index==nil then byte_index=0 end
		byte=self:GetByte(byte_index)
		return ((byte&0xf0)>>4)*10+byte&0x0f
	end,
	GetBit=function(self,byte_index,bit_index)
		--if byte_index==nil then byte_index=0 end
		if bit_index==nil then bit_index=0 end
		return (self:GetByte(byte_index)>>(bit_index))&0x01
	end,
	GetWord=function(self,byte_index,swap_byte)
		if byte_index==nil then byte_index=0 end
		if swap_byte then
			return (self:GetByte(byte_index)<<8)+self:GetByte(byte_index+1)
		else
			return self:GetByte(byte_index)+(self:GetByte(byte_index+1)<<8)
		end
	end,
	GetDWord=function(self,byte_index,swap_word,swap_byte)
		if byte_index==nil then byte_index=0 end
		if swap_word then
			if swap_byte then
				return self:GetByte(byte_index+3)+(self:GetByte(byte_index+2)<<8)+(self:GetByte(byte_index+1)<<16)+(self:GetByte(byte_index)<<24)
			else
				return self:GetByte(byte_index+2)+(self:GetByte(byte_index+3)<<8)+(self:GetByte(byte_index)<<16)+(self:GetByte(byte_index+1)<<24)
			end
		else
			if swap_byte then
				return self:GetByte(byte_index+1)+(self:GetByte(byte_index)<<8)+(self:GetByte(byte_index+3)<<16)+(self:GetByte(byte_index+2)<<24)
			else
				return self:GetByte(byte_index)+(self:GetByte(byte_index+1)<<8)+(self:GetByte(byte_index+2)<<16)+(self:GetByte(byte_index+3)<<24)
			end
		end
	end,
	GetShort=function(self,byte_index,swap_byte)
		if byte_index==nil then byte_index=0 end
		local short=self:GetWord(byte_index,swap_byte)
		if (short&0x8000)~=0 then
			short=-((short~0xffff)+1)
		end
		return short
	end,
	GetInt=function(self,byte_index,swap_word,swap_byte)
		if byte_index==nil then byte_index=0 end
		local int=self:GetDWord(byte_index,swap_word,swap_byte)
		if (int&0x80000000)~=0 then
			int=-((int~0xffffffff)+1)
		end
		return int
	end,
	GetFloat=function(self,byte_index,swap_word,swap_byte)
		if byte_index==nil then byte_index=0 end
		local dword=self:GetDWord(byte_index,swap_word,swap_byte)
		return self.DWordToFloat(dword)
	end,
	----------------------------------------------------------------------------------------------
	--设置Buffer方法
	SetByte=function(self,byte,byte_index)
		if byte==nil then byte=0 end
		if byte_index==nil then byte_index=0 end
		byte_index=self:ScalcByteIndex(byte_index)
		if byte_index>self.len then
			self.len=byte_index
		end
		self.arr[byte_index]=byte&0xff
	end,
	SetBCD=function(self,byte,byte_index)
		if byte==nil then byte=0 end
		--if byte_index==nil then byte_index01 end
		byte=((byte//10)<<4)+byte%10
		self:SetByte(byte,byte_index)
		return byte
	end,
	SetWord=function(self,word,byte_index,swap_byte)
		if word==nil then word=0 end
		if byte_index==nil then byte_index=1 end
		if swap_byte then
			self:SetByte(word>>8,byte_index)
			self:SetByte(word,byte_index+1)
		else
			self:SetByte(word,byte_index)
			self:SetByte(word>>8,byte_index+1)
		end
	end,
	SetDWord=function(self,dword,byte_index,swap_word,swap_byte)
		if dword==nil then dword=0 end
		if byte_index==nil then byte_index=0 end
		if swap_word then
			if swap_byte then
				self:SetByte(dword>>24,byte_index)
				self:SetByte(dword>>16,byte_index+1)
				self:SetByte(dword>>8,byte_index+2)
				self:SetByte(dword,byte_index+3)
			else
				self:SetByte(dword>>16,byte_index)
				self:SetByte(dword>>24,byte_index+1)
				self:SetByte(dword,byte_index+2)
				self:SetByte(dword>>8,byte_index+3)
			end
		else
			if swap_byte then
				self:SetByte(dword>>8,byte_index)
				self:SetByte(dword,byte_index+1)
				self:SetByte(dword>>24,byte_index+2)
				self:SetByte(dword>>16,byte_index+3)
			else
				self:SetByte(dword,byte_index)
				self:SetByte(dword>>8,byte_index+1)
				self:SetByte(dword>>16,byte_index+2)
				self:SetByte(dword>>24,byte_index+3)
			end
		end
	end,
	SetFloat=function(self,float,byte_index,swap_word,swap_byte)
		if float==nil then float=0 end
		if byte_index==nil then byte_index=0 end
		local dword=self.FloatToDWord(float)
		self:SetDWord(dword,byte_index,swap_word,swap_byte)
	end
}

function new_buffer(arr,narr)
	local buffer=setmetatable({},{__index=buffer_baseclass})
	return buffer:SetBuffer(arr,narr)
end
-------------------------------------------------------------------------
devscript_baseclass = {
	--接口
	OnInit=function(self)
	end,
	OnSend=function(self,poll_count)
	end,
	OnRecv=function(self,buffer,cid)
	end,
	--[[可用PollSend/PollSendProc代替
	OnTimer=function(self)
	end,
	]]
	OnYkSelect=function(self,yk_group,yk_onoff)
	end,
	OnYkExecute=function(self,yk_group,yk_onoff)
	end,
	OnYkCancel=function(self,yk_group,yk_onoff)
	end,
	OnSetTime=function(self)
	end,
	OnReset=function(self)
	end,
	--初始化
	PollSend=function(self,buffer,recv_proc,interval_sec)
	end,
	PollSendProc=function(self,send_proc,interval_sec)
	end,
	--获取属性
	GetDeviceNo=function(self)
	end,
	GetChannelNo=function(self)
	end,
	GetLinkAddr=function(self)
	end,
	GetLinkAddr2=function(self)
	end,
	GetParam1=function(self)
	end,
	GetParam2=function(self)
	end,
	--设置属性
	SetLinkAddr=function(self,addr)
	end,
	--数据操作
	GetYXOne=function(self,yx_index)
	end,
	GetYCOne=function(self,yc_index)
	end,
	MailYXOne=function(self,yx_index,yx_state)
	end,
	MailYXByte=function(self,yx_index,byte)
	end,
	MailYCOne=function(self,yc_index,yc_value)
	end,
	NewSOEUnit=function()
	end,
	MailSOE=function(self,soe_unit)
	end,
	MailYKResult=function(self,yk_result)
	end,
	--设备操作
	MakeFrame=function(self,cid,buffer,frame_type)
	end,
	Send=function(self,buffer,recv_proc)
	end,
	SetKeepSend=function(self,keep_sec)
	end,
	--调试
	DebugPrint=function(self,print_string)
	end
}
