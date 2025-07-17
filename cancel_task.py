# -*- coding: utf-8 -*-
import base64
import datetime
import hashlib
import hmac
import json
from urllib.parse import urlparse
import requests

# ==== Настройки ====
ApiAppKey = 'APIDxoVSCWkGetrlcQRTnh3cVD0NouLJuxW6hY4Ty'
ApiAppSecret = 'DjkTR3BMpCziGizXNJMre9VjbvZysSBNX7CKL'
task_id = 'trap dor Desti'  # <-- ВСТАВЬ СВОЙ task_id

# ==== Параметры запроса ====
url = "https://csg-open-platform.pudutech.com/pudu-entry/open-platform-service/v1/custom_call/cancel"
http_method = "POST"
content_type = "application/json"
accept = "application/json"
path = urlparse(url).path

# ==== Тело запроса ====
body = {
    "task_id": task_id,
    "call_device_name": ApiAppKey
}
body_json = json.dumps(body)

# ==== Корректный MD5 (Base64 от байтов digest, НЕ от hex строки!) ====
md5_digest = hashlib.md5(body_json.encode('utf-8')).digest()
content_md5 = base64.b64encode(md5_digest).decode()

# ==== x-date ====
x_date = datetime.datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")

# ==== Строка для подписи (StringToSign) ====
signing_str = f"x-date: {x_date}\n{http_method}\n{accept}\n{content_type}\n{content_md5}\n{path}"

# ==== Подпись HMAC SHA1 ====
signature = hmac.new(ApiAppSecret.encode('utf-8'), signing_str.encode('utf-8'), hashlib.sha1).digest()
signature_base64 = base64.b64encode(signature).decode()

# ==== Заголовок Authorization ====
auth_header = f'hmac id="{ApiAppKey}", algorithm="hmac-sha1", headers="x-date", signature="{signature_base64}"'

# ==== Заголовки ====
headers = {
    "Host": urlparse(url).hostname,
    "Accept": accept,
    "Content-Type": content_type,
    "x-date": x_date,
    "Authorization": auth_header,
    "Content-MD5": content_md5
}

# ==== Отправка запроса ====
response = requests.post(url, headers=headers, data=body_json)

# ==== Вывод результата ====
print("→ Статус запроса:", response.status_code)
print(response.text)
