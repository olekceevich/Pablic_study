import base64
import datetime
import hashlib
import hmac
from urllib.parse import urlparse, unquote
import requests

ApiAppKey = 'APIDiMW1UevdLbog1EKaLmBuVIQkwsOgLlJxEy'   #       APIDiMW1UevdLbog1EKaLmBuVIQkwsOgLlJxEy
ApiAppSecret = 'EMhMk3eJ9m3um8N5X9I9mp5qmT2xbmVjB7II3'                    # 

# URL для healthCheck
#Url = "https://open-platform.pudutech.com/pudu-entry/data-open-platform-service/v1/api/healthCheck"
Url = "https://csg-open-platform.pudutech.com/pudu-entry/data-open-platform-service/v1/api/healthCheck"


HTTPMethod = "GET"
Accept = "application/json"
ContentType = "application/json"

# Парсим путь
urlInfo = urlparse(Url)
Host = urlInfo.hostname
Path = urlInfo.path

# Убираем /release и т.д.
if Path.startswith(("/release", "/test", "/prepub")):
    Path = "/" + Path[1:].split("/", 1)[1]
Path = Path if Path else "/"

# Время
GMT_FORMAT = "%a, %d %b %Y %H:%M:%S GMT"
xDate = datetime.datetime.utcnow().strftime(GMT_FORMAT)

# Подпись
ContentMD5 = ""
signing_str = "x-date: %s\n%s\n%s\n%s\n%s\n%s" % (
    xDate,
    HTTPMethod,
    Accept,
    ContentType,
    ContentMD5,
    Path,
)

sign = hmac.new(ApiAppSecret.encode(), msg=signing_str.encode(), digestmod=hashlib.sha1).digest()
sign = base64.b64encode(sign).decode()
auth = f'hmac id="{ApiAppKey}", algorithm="hmac-sha1", headers="x-date", signature="{sign}"'

# Заголовки
headers = {
    "Host": Host,
    "Accept": Accept,
    "Content-Type": ContentType,
    "x-date": xDate,
    "Authorization": auth
}

# Запрос
response = requests.get(Url, headers=headers)

# Ответ
print(f"→ Код ответа: {response.status_code}")
print("→ Ответ от сервера:")
print(response.text)
