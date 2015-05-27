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
function buffer:GetUShort=GetWord
function buffer:GetShort(byte_index,swap_byte)
function buffer:GetDWord(byte_index,swap_word,swap_byte)
function buffer:GetUInt=GetDWord
function buffer:GetInt(byte_index,swap_word,swap_byte)
function buffer:GetFloat(byte_index,swap_word,swap_byte)
function buffer:GetDouble(byte_index)--赞不支持
--设置Buffer方法
function buffer:SetByte(byte_index,byte)
function buffer:SetBCD(byte_index,bcd_byte)
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
		arr=arr or {}
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
		byte_index=byte_index or 0
		return self.offset+byte_index
	end,
	PrintBuffer=function(self)
		print("\nPrintBuffer: len="..tostring(self.len).." offset="..tostring(self.offset).." bytes="..tostring(self.len-self.offset))
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
		byte_index=byte_index or 0
		byte_index=self:ScalcByteIndex(byte_index)
		if byte_index>=self.len or self.arr[byte_index]==nil then
			return 0
		end
		return self.arr[byte_index]&0xff
	end,
	GetBCD=function(self,byte_index)
		--byte_index=byte_index or 0
		byte=self:GetByte(byte_index)
		return ((byte&0xf0)>>4)*10+byte&0x0f
	end,
	GetBit=function(self,byte_index,bit_index)
		--byte_index=byte_index or 0
		if bit_index==nil then bit_index=0 end
		return (self:GetByte(byte_index)>>(bit_index))&0x01
	end,
	GetWord=function(self,byte_index,swap_byte)
		byte_index=byte_index or 0
		if swap_byte then
			return (self:GetByte(byte_index)<<8)+self:GetByte(byte_index+1)
		else
			return self:GetByte(byte_index)+(self:GetByte(byte_index+1)<<8)
		end
	end,
	GetShort=function(self,byte_index,swap_byte)
		byte_index=byte_index or 0
		local short=self:GetWord(byte_index,swap_byte)
		if (short&0x8000)~=0 then
			short=-((short~0xffff)+1)
		end
		return short
	end,
	GetDWord=function(self,byte_index,swap_word,swap_byte)
		byte_index=byte_index or 0
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
	GetInt=function(self,byte_index,swap_word,swap_byte)
		byte_index=byte_index or 0
		local int=self:GetDWord(byte_index,swap_word,swap_byte)
		if (int&0x80000000)~=0 then
			int=-((int~0xffffffff)+1)
		end
		return int
	end,
	GetFloat=function(self,byte_index,swap_word,swap_byte)
		byte_index=byte_index or 0
		local dword=self:GetDWord(byte_index,swap_word,swap_byte)
		local bin=string.pack("L",dword)
		local float=string.unpack("f",bin)
		return float
	end,
	----------------------------------------------------------------------------------------------
	--设置Buffer方法
	SetByte=function(self,byte_index,byte)
		byte=byte or 0
		byte_index=byte_index or 0
		byte_index=self:ScalcByteIndex(byte_index)
		if byte_index>=self.len then
			self.len=byte_index+1
		end
		self.arr[byte_index]=byte&0xff
	end,
	SetBCD=function(self,byte_index,bcd_byte)
		bcd_byte=bcd_byte or 0
		--byte_index=byte_index or 0
		bcd_byte=((bcd_byte//10)<<4)+bcd_byte%10
		self:SetByte(byte_index,bcd_byte)
		return bcd_byte
	end,
	SetWord=function(self,byte_index,word,swap_byte)
		word=word or 0
		byte_index=byte_index or 0
		if swap_byte then
			self:SetByte(byte_index,word>>8)
			self:SetByte(byte_index+1,word)
		else
			self:SetByte(byte_index,word)
			self:SetByte(byte_index+1,word>>8)
		end
	end,
	SetDWord=function(self,dword,byte_index,swap_word,swap_byte)
		dword=dword or 0
		byte_index=byte_index or 0
		if swap_word then
			if swap_byte then
				self:SetByte(byte_index,dword>>24)
				self:SetByte(byte_index+1,dword>>16)
				self:SetByte(byte_index+2,dword>>8)
				self:SetByte(byte_index+3,dword)
			else
				self:SetByte(byte_index,dword>>16)
				self:SetByte(byte_index+1,dword>>24)
				self:SetByte(byte_index+2,dword)
				self:SetByte(byte_index+3,dword>>8)
			end
		else
			if swap_byte then
				self:SetByte(byte_index,dword>>8)
				self:SetByte(byte_index+1,dword)
				self:SetByte(byte_index+2,dword>>24)
				self:SetByte(byte_index+3,dword>>16)
			else
				self:SetByte(byte_index,dword)
				self:SetByte(byte_index+1,dword>>8)
				self:SetByte(byte_index+2,dword>>16)
				self:SetByte(byte_index+3,dword>>24)
			end
		end
	end,
	SetFloat=function(self,byte_index,float,swap_word,swap_byte)
		float=float or 0
		byte_index=byte_index or 0
		local bin=string.pack("f",float)
		local dword=string.unpack("L",bin)
		self:SetDWord(dword,byte_index,swap_word,swap_byte)
	end
}
buffer_baseclass.GetUShort=buffer_baseclass.GetWord
buffer_baseclass.GetUInt=buffer_baseclass.GetDWord

function new_buffer(arr,narr)
	local buffer=setmetatable({},{__index=buffer_baseclass})
	return buffer:SetBuffer(arr,narr)
end
-------------------------------------------------------------------------
--[[
devscript_baseclass 类,脚本类的基类
备注：
soe_unit={year,month,day,hour,min,sec,ms,yx_index,yx_value,action_value}
]]
devscript_baseclass = {
	--接口
	--[[接口需要让用户自己定义
	OnInit=function(dev_inst)
	end,
	OnSend=function(dev_inst,poll_count)
	end,
	OnRecv=function(dev_inst,buffer,cmd)
	end,
	OnYkSelect=function(dev_inst,yk_group,yk_onoff)
	end,
	OnYkExecute=function(dev_inst,yk_group,yk_onoff)
	end,
	OnYkCancel=function(dev_inst,yk_group,yk_onoff)
	end,
	OnSetTime=function(dev_inst)
	end,
	OnReset=function(dev_inst)
	end,]]
	--[[可用PollSend/PollSendProc代替
	OnTimer=function(dev_inst)
	end,
	]]
	--获取属性
	GetDeviceNo=function(dev_inst)
	end,
	GetChannelNo=function(dev_inst)
	end,
	GetLinkAddr=function(dev_inst)
	end,
	GetLinkAddr2=function(dev_inst)
	end,
	GetParam1=function(dev_inst)
	end,
	GetParam2=function(dev_inst)
	end,
	--设置属性
	SetLinkAddr=function(dev_inst,addr)
	end,
	--数据操作
	GetYXOne=function(dev_inst,yx_index)
	end,
	GetYCOne=function(dev_inst,yc_index)
	end,
	MailYXOne=function(dev_inst,yx_index,yx_state)
	end,
	MailYXByte=function(dev_inst,yx_index,byte)
	end,
	MailYCOne=function(dev_inst,yc_index,yc_value)
	end,
	NewSOEUnit=function()
	end,
	MailSOE=function(dev_inst,soe_unit)
	end,
	MailYKResult=function(dev_inst,yk_result)
	end,
	--设备操作
	MakeFrame=function(dev_inst,cmd,buffer,frame_type)
	end,
	Send=function(dev_inst,buffer,recv_proc)
	end,
	PollSend=function(dev_inst,buffer,recv_proc,interval_sec)
	end,
	PollSendProc=function(dev_inst,send_proc,interval_sec)
	end,
	SetKeepSend=function(dev_inst,keep_sec)
	end,
	--调试
	DebugPrint=function(dev_inst,message)
	end
}
---------------------------------------------------------------------------
--global api
function PrintTable(t, indent, done)
	--print ( string.format ('PrintTable type %s', type(keys)) )
	if type(t) ~= "table" then return end
	print("\n PrintTable:")
	
	done = done or {}
	done[t] = true
	indent = indent or 0
	
	local l = {}
	for k, v in pairs(t) do
		table.insert(l, k)
	end
	
	table.sort(l)
	for k, v in ipairs(l) do
		local value = t[v]
		
		if type(value) == "table" and not done[value] then
			done [value] = true
			print(string.rep ("\t", indent)..tostring(v)..":")
			PrintTable (value, indent + 1, done)
		elseif type(value) == "userdata" and not done[value] then
			done [value] = true
			print(string.rep ("\t", indent)..tostring(v)..": "..tostring(value))
			PrintTable ((getmetatable(value) and getmetatable(value).__index) or getmetatable(value), indent + 2, done)
		else
			print(string.rep ("\t", indent)..tostring(v)..": "..tostring(value))
		end
	end
end
