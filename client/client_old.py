#!/usr/bin/env python
# -*- coding: UTF-8 -*- 
import socket
import select
client=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
client.connect(('166.111.82.233',6789)) #拨通电话
client.setblocking(False)
inputs = [client, ] #自己也要监测呀,因为server本身也是个fd
outputs = [client, ]
while True:
    print("waiting for next event...")
    readable, writeable, exceptional = select.select(inputs,outputs,inputs) #如果没有任何fd就绪,那程序就会一直阻塞在这里
    for s in writeable: #每个s就是一个socket
        if s is client:
            x = input()
            if x == '1':
                client.send('q\n'.encode('utf-8')) #发消息
                back_msg=client.recv(1024)
                print(back_msg)
            if x == '2':
                client.send('PASS qqq\n'.encode('utf-8')) #发消息
                back_msg=client.recv(1024)
                print(back_msg)
            if x == '3':
                client.send('PORT 192,168,11,131,10,0\n'.encode('utf-8')) #发消息
                back_msg=client.recv(1024)
                print(back_msg)
                filetrans=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
                filetrans.bind(('', 2560))
                filetrans.listen(1)
                inputs.append(filetrans)
            if x == '4':
                client.send('RETR \n'.encode('utf-8')) #发消息 
                back_msg=client.recv(1024)
                print(back_msg)
    for s in readable:   
        conn, addr = s.accept()
        print("connected by", addr)
        conn.recv(1024)
        print("data")










# client.send('PASV\n'.encode('utf-8')) #发消息
# back_msg=client.recv(1024)
# print(back_msg)
# back_msg = back_msg.decode()
# ip = "192.168.11.131"
# port = int(back_msg[:-2])
# print(port)


# client.send('RETR \n'.encode('utf-8')) #发消息
# back_msg=client.recv(1024)
# print(back_msg)

# filetrans=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
# filetrans.connect(('192.168.11.131', port))
# msg= filetrans.recv(1024)
# print("cccc,", msg)


client.close()