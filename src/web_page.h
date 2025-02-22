#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

// ホームページ
const char HTML_HOME[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <meta lang='ja'>
    <head>
      <meta charset='utf-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1'>
      <title>ホーム</title>
    </head>
    <body>
      <h1>設定メニュー</h1>
      <ul>
        <li><a href='/led'>LEDの設定</a></li>
        <li><a href='/calendar'>日付の設定</a></li>
      </ul>
    </body>
    </html>
    )rawliteral";
    
    // LED設定ページ
    const char HTML_LED[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <meta lang='ja'>
    <head>
      <meta charset='utf-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1'>
      <title>LED設定</title>
    </head>
    <body>
      <h1>LED点滅設定</h1>
      <form action='/led' method='get'>
        点滅回数: <input type='number' name='count' min='1' max='20'><br>
        <p>%MESSAGE%</p>
        <h3>暗証番号設定</h3>
        ※暗証番号未設定のカードは登録せず利用可能です<br>
        ドライバー1: <input type='number' name='drv1' min='0' max='9999'><br>
        ドライバー2: <input type='number' name='drv2' min='0' max='9999'><br>
        ドライバー3: <input type='number' name='drv3' min='0' max='9999'><br>
        <br>
        いたずら防止用暗証番号：<input type='number' name='secno' min='0' max='9999'><br>
        arduino_secrets.h に設定した SECURITY_NO を入力します。<br>
        <input type='submit' value='設定'>
      </form>
      ※セキュリティ上、登録されている暗証番号はワイヤレスでは表示していません。<br>
      ArduinoをPCにUSB接続し、シリアルモニタを表示しながら本ページを開くと出力されます。<br>
      <br>
      <a href='/'>← ホームへ戻る</a>
    </body>
    </html>
    )rawliteral";

    // カレンダー設定ページ
    const char HTML_CALENDAR[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <meta lang='ja'>
    <head>
      <meta charset='utf-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1'>
      <title>カレンダー設定</title>
    </head>
    <body>
      <h1>カレンダー設定</h1>
      本体内部のカレンダーを設定します。<br>

      <h3>本体のカレンダー</h3>
      <b>%RTC_Y% 年 %RTC_M% 月 %RTC_D% 日</b> 

      <h3>設定の変更</h3>
      <form action='/calendar' method='get'>
        <input type='date' name='date' min='2025-01-01' max='2090-12-31'>
        <br>
        スマホの日付がデフォルト入力されています。<br>
        期限より未来の日付に設定すると動作テストができます。<br>
        時刻はスマホの時計から取得されます。<br>
        <br>

        <input type='submit' value='設定'>
      </form>
      <br>
      <a href='/'>← ホームへ戻る</a>
    </body>
    <script defer>
      document.addEventListener("DOMContentLoaded", function () {
          let today = new Date();
          let formattedDate = today.toISOString().split('T')[0];
          let dateInputs = document.getElementsByName("date");
          if (dateInputs.length > 0) {
              dateInputs[0].value = formattedDate;
          }
      });
  </script>
    </html>
    )rawliteral";



#endif