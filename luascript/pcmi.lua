print("LoadDevScript")

------------------------------------------------------------------------
DevScript = {frame_model="modbus.so"}
DevScript.gc_count=0
function DevScript:OnInit()
	self:DebugPrint("this is OnInit")
	self.poll_type=1
	local SendBuffer=self:MakeFrame(0x03,new_buffer{0x00,0x00,0x00,0x25})
	SendBuffer:PrintBuffer()
	self:PollSend(SendBuffer)
	self:PollSendProc("OnSetTime",600)
end

function DevScript:OnSend()
	local scriptclass_metatable=getmetatable(self)
	local scriptclass=scriptclass_metatable.__index
	scriptclass.gc_count=scriptclass.gc_count+1
	if (scriptclass.gc_count>=300) then
		scriptclass.gc_count=0
--		collectgarbage()
	end
	self:DebugPrint("gc_count="..scriptclass.gc_count)
	self:DebugPrint("garbage count="..collectgarbage("count")*1024)
	self:DebugPrint("this is OnSend")
	self.poll_type=(self.poll_type+1)%2
	self:DebugPrint("poll_type="..tostring(self.poll_type))
	if self.poll_type==1 then
		--local SendBuffer=self:MakeFrame(0x03,new_buffer({0x00,0x00,0x00,0x25}))
		--local SendBuffer=new_buffer({0x01,0x03,0x00,0x00,0x00,0x25,0x84,0x11})
		--self:Send(SendBuffer,nil,false)
		return false
	else
		local SendBuffer=new_buffer{0x01,0x03,0x03,0xe8,0x00,0x02,0x44,0x7b}
		self:Send(SendBuffer,"OnExplainYx")
	end
	return true
end
function DevScript:OnExplainYx(buffer)
	self:DebugPrint("this is OnExplainYx")
	if buffer.len<2 then
		return false
	end
	self:MailYXByte(2,buffer:GetByte(0))
	self:MailYXByte(10,buffer:GetByte(1))
	return true
end

function DevScript:OnRecv(buffer)
	self:DebugPrint("this is OnRecv")
	--buffer:PrintBuffer()
	local num=buffer.len//2
	for i = 0,num-1 do
		self:MailYCOne(i,buffer:GetWord(i*2,true))
		--self:DebugPrint("MailYc "..i.." "..buffer:GetWord(i*2,true))
	end
	print("\n clock:"..os.clock())
	return true
end

function DevScript:OnExplainSoe(buffer)
	self:DebugPrint("this is OnExplainSoe")
	if (nbuffer~=10) then return false end
	local soe={}
	--不填则缺省为当前时间
	soe.year=buffer:GetByte(0)
	soe.month=buffer:GetByte(1)
	soe.day=buffer:GetByte(2)
	soe.hour=buffer:GetByte(3)
	soe.min=buffer:GetByte(4)
	soe.sec=buffer:GetByte(5)
	soe.ms=buffer:GetByte(6)
	soe.yx_point=buffer:GetByte(7)
	soe.yx_state=buffer:GetByte(8)
	soe.action_value=buffer:GetByte(9)
	self:MailSOE(soe)
	return true
end

function DevScript:OnYkSelect(yk_group, yk_onoff)
	self:DebugPrint("this is OnYkSelect")
	local regaddr=0x0000+yk_group
	local regdata=0x0000
	if (not yk_onoff) then
		regdata=0xffff
	end
	local SendBuffer=new_buffer()
	SendBuffer:SetWord(0,true,regaddr)
	SendBuffer:SetWord(2,true,regdata)
	self:Send(self:MakeFrame(0x10,SendBuffer))
	return true
end
function DevScript:OnYkExecute(yk_group, yk_onoff)
	self:DebugPrint("this is OnYkExecute")
	local regaddr=0x0000+yk_group
	local regdata=0x0000
	if (not yk_onoff) then
		regdata=0xffff
	end
	local SendBuffer=new_buffer()
	SendBuffer:SetWord(0,true,regaddr)
	SendBuffer:SetWord(2,true,regdata)
	self:Send(self:MakeFrame(0x10,SendBuffer))
	return true
end

function DevScript:OnSetTime()
	self:DebugPrint("this is OnSetTime")
	local SendBuffer = new_buffer()
	local nowdate=os.date("*t")
	SendBuffer:SetBCD(0,nowdate.year%100)
	SendBuffer:SetBCD(1,nowdate.month)
	SendBuffer:SetBCD(2,nowdate.day)
	SendBuffer:SetBCD(3,nowdate.hour)
	SendBuffer:SetBCD(4,nowdate.min)
	SendBuffer:SetBCD(5,nowdate.sec)
	SendBuffer=self:MakeFrame(0x10,SendBuffer)
	self:Send(SendBuffer)
	return true
end
