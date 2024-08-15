import requests
import json
import time

# 主控的URL和头信息
url = "http://192.168.123.171/json/state"
headers = {"Content-Type": "application/json"}

NumLeds=100



def lightOneLed(ledIndex:int):
    # 将一个灯珠的亮度设置为最大值255
    data = {"seg": [{"start": 0, "stop": NumLeds, "bri": 0}, {"start": ledIndex, "stop": ledIndex+1, "bri": 255}]}
    return json.dumps(data)
    
def lightOneLedWithTail(ledIndex,tailLength:int):
    # 将一个灯珠的亮度设置为最大值255，附带拖尾效果
    if type(ledIndex) is int:
        data = {"seg": [{"start": 0, "stop": NumLeds, "bri": 0}, {"start": ledIndex, "stop": ledIndex+tailLength, "bri": 255}]}
    if type(ledIndex) is list:
        data = {"seg": [{"start": 0, "stop": NumLeds, "bri": 0}, 
                        *[{"start": ledInde, "stop": ledInde+tailLength, "bri": 255} for ledInde in ledIndex]
                        ]}
        print(data)
    assert data is not None
    return json.dumps(data)
    

# 首先让所有灯按暗掉
# 初始化所有的灯珠亮度为0
data = {"seg": []}
requests.post(url, headers=headers, data=json.dumps(data), timeout=1)

# 1. 一个灯珠从最开始移动到第10位，模拟烟花飞上天空，移动速度0.1s/led
上升高度=10
if True:
    for i in range(上升高度+1):
        
        # 发送POST请求更新灯带状态
        response = requests.post(url, headers=headers, data=lightOneLed(i), timeout=1)
        
        # 检查响应状态码
        if response.status_code == 200:
            print("Request successful.")
        else:
            print(f"Error on sending POST: {response.status_code}")
        
        # 控制速度：暂停0.1秒
        time.sleep(0.1)

# 2. 3个5灯珠灯带，模拟烟花炸开效果
# 定义烟花炸开的位置
fireworkStart = 上升高度+2

瓣数量=3
瓣长度=5

# for bloomIndex in range(瓣长度):  
# 发送POST请求更新灯带状态
response = requests.post(url, headers=headers, data=lightOneLedWithTail([fireworkStart + i * 瓣长度 for i in range(瓣数量)],2), timeout=1)

# 检查响应状态码
if response.status_code == 200:
    print("Request successful.")
