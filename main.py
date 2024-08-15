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
    
    

# 首先让所有灯按暗掉
# 初始化所有的灯珠亮度为0
data = {"seg": [{"start": 0, "stop": NumLeds, "bri": 0}]}
requests.post(url, headers=headers, data=json.dumps(data), timeout=1)

# 遍历从第一个灯珠到最后一个灯珠
for i in range(21):
    
    # 发送POST请求更新灯带状态
    response = requests.post(url, headers=headers, data=lightOneLed(i), timeout=1)
    
    # 检查响应状态码
    if response.status_code == 200:
        print("Request successful.")
    else:
        print(f"Error on sending POST: {response.status_code}")
    
    # 控制速度：暂停0.2秒
    time.sleep(0.2)

# 