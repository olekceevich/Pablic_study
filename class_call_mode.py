# pudu_api_client.py
import base64
import datetime
import hashlib
import hmac
import json
import time
from urllib.parse import urlparse, quote, unquote
import requests
from typing import Optional, Dict, Any

class ApiAuth:
    def __init__(self, app_key: str, app_secret: str):
        self.app_key = app_key
        self.app_secret = app_secret

    def sign_request(self, method: str, url: str, content_type: str, accept: str, body: Optional[str] = "") -> Dict[str, str]:
        url_info = urlparse(url)
        host = url_info.hostname
        path = url_info.path

        if path.startswith(('/release', '/test', '/prepub')):
            path = '/' + path[1:].split('/', 1)[1]
        path = path or '/'

        content_md5 = base64.b64encode(hashlib.md5(body.encode()).hexdigest().encode()).decode() if body else ""
        x_date = datetime.datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")

        signing_str = f"x-date: {x_date}\n{method}\n{accept}\n{content_type}\n{content_md5}\n{path}"
        signature = hmac.new(self.app_secret.encode(), signing_str.encode(), hashlib.sha1).digest()
        signature_b64 = base64.b64encode(signature).decode()
        auth_header = f'hmac id="{self.app_key}", algorithm="hmac-sha1", headers="x-date", signature="{signature_b64}"'

        return {
            "Host": host,
            "Accept": accept,
            "Content-Type": content_type,
            "x-date": x_date,
            "Authorization": auth_header,
            "Content-MD5": content_md5
        }


class PuduAPIClient:
    def __init__(self, app_key: str, app_secret: str, base_url: str):
        self.auth = ApiAuth(app_key, app_secret)
        self.base_url = base_url.rstrip('/')
        self.app_key = app_key

    def _post(self, path: str, payload: dict) -> requests.Response:
        url = f"{self.base_url}{path}"
        body_json = json.dumps(payload)
        headers = self.auth.sign_request("POST", url, "application/json", "application/json", body_json)
        return requests.post(url, headers=headers, data=body_json)

    def call_robot(self, sn: str, map_name: str, point: str, point_type: str = "table", call_mode: str = "CALL") -> requests.Response:
        payload = {
            "sn": sn,
            "map_name": map_name,
            "point": point,
            "call_device_name": self.app_key,
            "point_type": point_type,
            "call_mode": call_mode
        }
        return self._post("/pudu-entry/open-platform-service/v1/custom_call", payload)

    def cancel_call(self, task_id: str) -> requests.Response:
        payload = {
            "task_id": task_id,
            "call_device_name": self.app_key
        }
        return self._post("/open-platform-service/v1/custom_call/cancel", payload)

    def complete_call(self, task_id: str, next_call_task: Optional[dict] = None) -> requests.Response:
        payload = {
            "task_id": task_id,
            "call_device_name": self.app_key
        }
        if next_call_task:
            payload["next_call_task"] = next_call_task
        print("\n🔄 Отправляем complete_call с:")
        print(json.dumps(payload, indent=2, ensure_ascii=False))
        return self._post("/pudu-entry/open-platform-service/v1/custom_call/complete", payload)


if __name__ == "__main__":
    APP_KEY = "APIDxoVSCWkGetrlcQRTnh3cVD0NouLJuxW6hY4Ty"
    APP_SECRET = "DjkTR3BMpCziGizXNJMre9VjbvZysSBNX7CKL"
    ROBOT_SN = "8170G5426060002"
    MAP_NAME = "#0#0silpoNEW"    ##0#0Destination tast     #0#0Map 3

    # Точка 1
    FIRST_POINT = "des1"    #igor Desti
    # Точка 2
    NEXT_POINT = "des2" \
    ""

    client = PuduAPIClient(APP_KEY, APP_SECRET, "https://csg-open-platform.pudutech.com")

    # Вызов на первую точку
    response = client.call_robot(sn=ROBOT_SN, map_name=MAP_NAME, point=FIRST_POINT, call_mode="DELIVERY")
    print(f"→ Вызов 1 ({FIRST_POINT}) — код: {response.status_code}")
    print(response.text)

    try:
        task_id = response.json()["data"]["task_id"]
    except Exception as e:
        print(" Не удалось получить task_id:", e)
        exit(1)

    input("\n Нажмите Enter, когда робот завершит задачу и будет готов к следующей точке...\n")

    next_task = {
        "map_name": MAP_NAME,
        "point": NEXT_POINT,
        "point_type": "table",
        "call_device_name": APP_KEY,
        "call_mode": "CALL"
    }

    response2 = client.complete_call(task_id=task_id, next_call_task=next_task)
    print(f"→ Вызов 2 ({NEXT_POINT}) — код: {response2.status_code}")
    print(response2.text)
