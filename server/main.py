from selenium import webdriver
from selenium.webdriver.firefox.options import Options as FirefoxOptions
from selenium.webdriver.common.by import By

import time
from dotenv import load_dotenv
import os

from flask import Flask

load_dotenv()

DEV = os.environ.get("ENV") == "development"
app = Flask(__name__)

if DEV:
    from selenium.webdriver.firefox.options import Options as FirefoxOptions
else:
    from selenium.webdriver.chrome.service import Service
    from selenium.webdriver.chrome.options import Options as ChromeOptions

options = FirefoxOptions() if DEV else ChromeOptions()
driver = None

if not DEV:
    options.binary_location = "/usr/bin/chromium-browser"
options.add_argument("--headless")
options.add_argument("--no-sandbox")
options.add_argument("--disable-dev-shm-usage")

if DEV:
    print("--- Using Firefox (DEV)\n")
    driver = webdriver.Firefox(options=options)
else:
    print("--- Using Chrome (PROD)\n")
    chrome_driver_path = "/usr/bin/chromedriver"
    service = Service(executable_path=chrome_driver_path)
    driver = webdriver.Chrome(service=service, options=options)


def get_code_with_auth():
    client_id = os.environ.get("SPOTIFY_CLIENT_ID")
    callback_uri = os.environ.get("SPOTIFY_CALLBACK_URI")
    if client_id is None or callback_uri is None:
        print("Error: No client id or callback uri")
        return None

    spotify_username = os.environ.get("SPOTIFY_EMAIL")
    spotify_password = os.environ.get("SPOTIFY_PASSWORD")
    if spotify_username is None or spotify_password is None:
        print("Error: No username or password")
        return None

    try:
        url = "https://accounts.spotify.com/en/login"
        driver.get(url)

        username_input = driver.find_element(By.ID, "login-username")
        username_input.send_keys(spotify_username)

        password_input = driver.find_element(By.ID, "login-password")
        password_input.send_keys(spotify_password)

        login_button = driver.find_element(By.ID, "login-button")
        login_button.click()

    except Exception as e:
        print("--- Error when logging in")
        print(e)
        driver.quit()
        return None

    time.sleep(1)

    try:
        url = f"https://accounts.spotify.com/authorize?client_id={client_id}&response_type=code&redirect_uri={callback_uri}&scope=playlist-modify-private%20playlist-modify-public"
        driver.get(url)
    except Exception as e:
        print("--- Error when redirecting to Spotify")
        print(e)
        driver.quit()
        return None

    time.sleep(1)

    code = driver.current_url.split("code=")[1].split("&")[0]
    driver.quit()

    return code


@app.get("/health")
def handle_health():
    return "OK"


@app.get("/callback")
def handle_callback():
    return {"message": "callback received"}


@app.get("/code")
def handle_get_code():
    code = get_code_with_auth()
    if code is None:
        print("--- Error: No code")
        return {"code": ""}

    if DEV:
        print(code)
    return {"code": code}


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=3000)
