import requests
import json
import time

# 主控的URL和头信息
url = "http://192.168.31.242/json/state"
headers = {"Content-Type": "application/json"}

NumLeds=100

上升高度=60
瓣数量=3
瓣长度=10
拖尾长度=2
光纤数量=5
# 首先让所有灯按暗掉
# 初始化所有的灯珠亮度为0
data = {"seg": [{"bri": 0}]*4}
requests.post(url, headers=headers, data=json.dumps(data), timeout=1)

# 1. 一个灯珠从最开始移动到第10位，模拟烟花飞上天空，移动速度0.1s/led

def lightOneLed(ledIndex:int):
    # 将一个灯珠的亮度设置为最大值255
    data = {"seg": [{"id": 0,"start": 0, "stop": NumLeds, "bri": 0}, {"id": 1,"start": ledIndex, "stop": ledIndex+1, "bri": 255, "n":"1-lift"}]}
    return json.dumps(data)

for i in range(上升高度+1):
    
    # 发送POST请求更新灯带状态
    response = requests.post(url, headers=headers, data=lightOneLed(上升高度-i), timeout=1)
    
    # 检查响应状态码
    if response.status_code == 200:
        print("Request successful.")
    else:
        print(f"Error on sending POST: {response.status_code}")
    
    # 控制速度：暂停0.1秒
    time.sleep(0.03)

time.sleep(0.1)
# 2. 3个5灯珠灯带，模拟烟花炸开效果
# 定义烟花炸开的位置
fireworkStart = 上升高度+1


# 2.5 同时，点亮光纤(假设有5条)
guangqianStart=fireworkStart+瓣长度*瓣数量
data = {"seg": [{"id": 4, "bri": 255 ,"start": guangqianStart, "stop": guangqianStart+光纤数量,"n":"guangqian"}]}
requests.post(url, headers=headers, data=json.dumps(data), timeout=1)

def lightOneLedWithTail(ledIndex,tailLength:int):
    # 将一个灯珠的亮度设置为最大值255，附带拖尾效果
    # if type(ledIndex) is int:
    #     data = {"seg": [{"start": 0, "stop": NumLeds, "bri": 0}, {"start": ledIndex, "stop": ledIndex+1+tailLength, "bri": 255}]}
    if type(ledIndex) is list:
        data = {"seg": [{"id": 0, "start": 0, "stop": NumLeds, "bri": 0}, 
                        *[{"start": ledInde-tailLength, "stop": ledInde+1, "bri": 255, "n": f"2-subbloom{i}"} for i,ledInde in enumerate(ledIndex)]
                        ]}
        print(data)
    assert data is not None
    return json.dumps(data)
    
for bloomIndex in range(瓣长度):  
    if bloomIndex<拖尾长度:
        data=lightOneLedWithTail([fireworkStart+bloomIndex + i * 瓣长度 for i in range(瓣数量)],bloomIndex)
    else:
        data=lightOneLedWithTail([fireworkStart+bloomIndex + i * 瓣长度 for i in range(瓣数量)],拖尾长度)

    # 发送POST请求更新灯带状态
    response = requests.post(url, headers=headers, data=data, timeout=1)

    # 检查响应状态码
    if response.status_code == 200:
        print("Request successful.")
    
    # input(".....")
    time.sleep(0.3)
# 到头之后拖尾后处理
for tuo in range(拖尾长度,-2,-1):
    data=lightOneLedWithTail([fireworkStart+瓣长度-1 + i * 瓣长度 for i in range(瓣数量)],tuo)
    response = requests.post(url, headers=headers, data=data, timeout=1)


