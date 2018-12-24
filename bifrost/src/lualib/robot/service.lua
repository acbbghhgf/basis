local service = {}

service.templates = {
    ["dlg"] = [[local inst = new($CFN)
if not inst then
    print("inst not found")
    write("PUT", $ETOPIC, [=[{"errId":50000,"error":"get inst failed"}]=])
else
    inst:calc($PARAM)
    local result = inst:getResult()
    if not result then
        write("PUT", $ETOPIC, [=[{"errId":50001,"error":"get result failed"}]=])
        inst:exit()
    else
        write("PUT", $WTOPIC, result)
        inst:close()
    end
end]],
}


return service
