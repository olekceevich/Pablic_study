# -*- coding: utf-8 -*-
import base64
import datetime
import hashlib
import hmac
import json
from urllib.parse import urlparse
import requests

# === Константы
ApiAppKey = 'APIDxoVSCWkGetrlcQRTnh3cVD0NouLJuxW6hY4Ty'
ApiAppSecret = 'DjkTR3BMpCziGizXNJMre9VjbvZysSBNX7CKL'
robot_sn = '8170G5426060002'

# === Настройки запроса
HTTPMethod = "GET"
Accept = "application/json"
ContentType = "application/json"
url = f"https://csg-open-platform.pudutech.com/pudu-entry/map-service/v1/open/current?sn={robot_sn}&need_element=true"

# === Расчёт Content-MD5 (пустое тело для GET → пустой md5)
body_json = ""
body_md5 = hashlib.md5(body_json.encode()).hexdigest()
ContentMD5 = base64.b64encode(body_md5.encode()).decode()

# === Временная метка
GMT_FORMAT = "%a, %d %b %Y %H:%M:%S GMT"
xDate = datetime.datetime.utcnow().strftime(GMT_FORMAT)

# === Парсинг пути
urlInfo = urlparse(url)
Host = urlInfo.hostname
Path = urlInfo.path

# === Строка для подписи
signing_str = "x-date: %s\n%s\n%s\n%s\n%s\n%s" % (
    xDate,
    HTTPMethod,
    Accept,
    ContentType,
    ContentMD5,
    Path,
)

# === Подпись HMAC
sign = hmac.new(ApiAppSecret.encode(), msg=signing_str.encode(), digestmod=hashlib.sha1).digest()
sign = base64.b64encode(sign).decode()
auth = f'hmac id="{ApiAppKey}", algorithm="hmac-sha1", headers="x-date", signature="{sign}"'

# === Заголовки
headers = {
    "Host": Host,
    "Accept": Accept,
    "Content-Type": ContentType,
    "x-date": xDate,
    "Authorization": auth,
    "Content-MD5": ContentMD5
}

# === Отправка запроса
response = requests.get(url, headers=headers)

print("Статус:", response.status_code)
print("Ответ:")

try:
    data = response.json()
    if "data" in data:
        print("🗺 Карта:", data["data"].get("name", ""))
        for elem in data["data"].get("elements", []):
            print(f"[{elem.get('mode')}] {elem.get('name')} → id: {elem.get('id')} → vector: {elem.get('vector')}")
    else:
        print("⚠️ Нет поля 'data'. Ответ:", data)
except Exception as e:
    print("❌ Ошибка разбора ответа:", e)
    print(response.text)
