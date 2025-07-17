# -*- coding: utf-8 -*-
import base64
import datetime
import hashlib
import hmac
import json
from urllib.parse import urlparse, quote, unquote
import requests

# ======================== Macros
ApiAppKey = 'APIDxoVSCWkGetrlcQRTnh3cVD0NouLJuxW6hY4Ty'   #       APIDiMW1UevdLbog1EKaLmBuVIQkwsOgLlJxEy
ApiAppSecret = 'DjkTR3BMpCziGizXNJMre9VjbvZysSBNX7CKL'                    # 
robot_sn = '8170G5426060002'                      # серийный номер робота
map_name = '#0#0Destination tast'   # имя карты    #0#0Destination tast
target_point = 'igor out'      #  igor out  igor Desti   ch     точка назначения test zona2
call_mode = 'DELIVERY'                                        # тип вызова: CALL, DELIVERY и т.д.

# Основной API-адрес (реальный для Европы):
Url = "https://csg-open-platform.pudutech.com/pudu-entry/open-platform-service/v1/custom_call"
HTTPMethod = "POST"
Accept = "application/json"
ContentType = "application/json"

# Сборка тела запроса
body = {
    "sn": robot_sn,
    "map_name": map_name,
    "point": target_point,
    "call_device_name": ApiAppKey,   # AppKey передаётся как идентификатор вызывающего устройства
    "point_type": "table",
    "call_mode": call_mode
}
body_json = json.dumps(body)

# Расчёт Content-MD5
body_md5 = hashlib.md5(body_json.encode()).hexdigest()
ContentMD5 = base64.b64encode(body_md5.encode()).decode()

# Парсинг пути из URL
urlInfo = urlparse(Url)
Host = urlInfo.hostname
Path = urlInfo.path
if Path.startswith(("/release", "/test", "/prepub")):
    Path = "/" + Path[1:].split("/", 1)[1]
Path = Path if Path else "/"

# Временная метка в формате GMT
GMT_FORMAT = "%a, %d %b %Y %H:%M:%S GMT"
xDate = datetime.datetime.utcnow().strftime(GMT_FORMAT)

# Строка для подписи (string-to-sign)
signing_str = "x-date: %s\n%s\n%s\n%s\n%s\n%s" % (
    xDate,
    HTTPMethod,
    Accept,
    ContentType,
    ContentMD5,
    Path,
)

# Генерация подписи (HMAC-SHA1 + Base64)
sign = hmac.new(ApiAppSecret.encode(), msg=signing_str.encode(), digestmod=hashlib.sha1).digest()
sign = base64.b64encode(sign).decode()
auth = f'hmac id="{ApiAppKey}", algorithm="hmac-sha1", headers="x-date", signature="{sign}"'

# Заголовки запроса
headers = {
    "Host": Host,
    "Accept": Accept,
    "Content-Type": ContentType,
    "x-date": xDate,
    "Authorization": auth,
    "Content-MD5": ContentMD5
}

# Отправка запроса
response = requests.post(Url, headers=headers, data=body_json)

# Печать результата
print(f"→ Код ответа: {response.status_code}")
print("→ Ответ от сервера:")
print(response.text)
