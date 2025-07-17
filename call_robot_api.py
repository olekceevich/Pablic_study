# -*- coding: utf-8 -*-
import base64
import datetime
import hashlib
import hmac
import json
import time
from urllib.parse import urlparse
import requests

# ======= НАСТРОЙКИ =======
ApiAppKey = 'APIDxoVSCWkGetrlcQRTnh3cVD0NouLJuxW6hY4Ty'   # Уникальный ключ приложения (API App Key)
ApiAppSecret = 'DjkTR3BMpCziGizXNJMre9VjbvZysSBNX7CKL'    # Секрет приложения (API App Secret)
robot_sn = '8170G5426060002'  # Серийный номер робота
map_name = '#0#0Destination tast'  # Имя карты, на которой находятся точки
point1 = 'igor Desti'             # начальная точка вызова
point2 = 'igor out'               # вторая точка
point3 = "trap dor Desti"         # третья точка (замени при необходимости)

# ======= ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ДЛЯ ГЕНЕРАЦИИ ЗАГОЛОВКОВ =======
def generate_headers(url, body_json, method="POST"):
    body_md5 = hashlib.md5(body_json.encode()).hexdigest()
    content_md5 = base64.b64encode(body_md5.encode()).decode()
    x_date = datetime.datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")
    path = urlparse(url).path or "/"
    
    # Формируем строку для подписи согласно требованиям API
    signing_str = f"x-date: {x_date}\n{method}\napplication/json\napplication/json\n{content_md5}\n{path}"
    
    # Создаём HMAC-подпись
    sign = hmac.new(ApiAppSecret.encode(), signing_str.encode(), hashlib.sha1).digest()
    sign_b64 = base64.b64encode(sign).decode()
    
    # Строка авторизации
    auth = f'hmac id="{ApiAppKey}", algorithm="hmac-sha1", headers="x-date", signature="{sign_b64}"'

    # Финальные заголовки
    return {
        "Host": urlparse(url).hostname,
        "Accept": "application/json",
        "Content-Type": "application/json",
        "x-date": x_date,
        "Authorization": auth,
        "Content-MD5": content_md5
    }

# ======= вызов робота на первую точку 
url1 = "https://csg-open-platform.pudutech.com/pudu-entry/open-platform-service/v1/custom_call"
body1 = {
    "sn": robot_sn,
    "map_name": map_name,
    "point": point1,
    "call_device_name": ApiAppKey,
    "point_type": "table",
    "call_mode": "DELIVERY"
}
body1_json = json.dumps(body1)
headers1 = generate_headers(url1, body1_json)
response1 = requests.post(url1, headers=headers1, data=body1_json)

print("→ Вызов на точку 1:", response1.status_code)
print(response1.text)
# =======  пауза и получение
task_id = None
try:
    task_id = response1.json().get("data", {}).get("task_id")
except:
    print(" task_id не получен из response1")

if not task_id:
    exit("Ошибка task_id отсутствует — завершение невозможно")

print(f"Получен task_id: {task_id}")
print("25 секунд перед следующим вызовом...")
#time.sleep(25)

# =======  Завершение задачи и вызов на вторую точку
url2 = "https://csg-open-platform.pudutech.com/pudu-entry/open-platform-service/v1/custom_call/complete"
body2 = {
    "task_id": task_id,
    "call_device_name": ApiAppKey,
    "next_call_task": {
        "map_name": map_name,
        "point": point2,
        "point_type": "table",
        "call_device_name": ApiAppKey,
        "call_mode": "CALL",  # режим без подтверждения
        "mode_data": {
            "switch_time": 0,       # сразу
            "cancel_btn_time": -1,  # не показывать кнопку отмены
            "show_timeout": 1       # показывать интерфейс 5 секунд
        }
    }
}
body2_json = json.dumps(body2)
headers2 = generate_headers(url2, body2_json)
response2 = requests.post(url2, headers=headers2, data=body2_json)

print("→Завершение задачи и вызов на точку 2:", response2.status_code)
print(response2.text)

# ======= Получение нового task_id из ответа response2
new_task_id = None
try:
    new_task_id = response2.json().get("data", {}).get("task_id")
except:
    print(" Новый task_id не получен из response2")

if not new_task_id:
    exit(" Ошибка: новый task_id отсутствует — нельзя продолжать")

print(f"Новый task_id: {new_task_id}")
print("25 секунд перед переходом на точку 3...")
#time.sleep(25)

# ====== Завершение задачи и вызов на третью точку 
url3 = "https://csg-open-platform.pudutech.com/pudu-entry/open-platform-service/v1/custom_call/complete"
body3 = {
    "task_id": new_task_id,
    "call_device_name": ApiAppKey,
    "next_call_task": {
        "map_name": map_name,
        "point": point3,
        "point_type": "table",
        "call_device_name": ApiAppKey,
        "call_mode": "CALL",
        "mode_data": {
            "switch_time": 0,
            "cancel_btn_time": -1,
            "show_timeout": 1
        }
    }
}
body3_json = json.dumps(body3)
headers3 = generate_headers(url3, body3_json)
response3 = requests.post(url3, headers=headers3, data=body3_json)

print("Завершение задачи и вызов на точку 3:", response3.status_code)
print(response3.text)
