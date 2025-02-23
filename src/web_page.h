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
      <ul style='font-size: 1.5em;'>
        <li><a href='/led'>LEDの設定</a></li>
        <li><a href='/calendar'>日付の設定</a></li>
        <li><a href='/endsetting'>設定を終了し再起動</a></li>
      </ul>
    </body>
    </html>
    )rawliteral";
    

    // 設定終了ページ
    const char HTML_ENDSETTING[] PROGMEM = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <meta lang='ja'>
      <head>
        <meta charset='utf-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
        <title>設定完了</title>
      </head>
      <body>
        <h1>設定完了</h1>
        本体を再起動します。<br>
        ブラウザを閉じて終了してください。<br>

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
        <input type='submit' value='設定' style='font-size: 1.5em; padding:0.5em; width:100%; max-width:20em;'>
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
      <html style='max-width: 480px; margin:auto;'>
      <meta lang='ja'>
      <head>
        <meta charset='utf-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
        <title>カレンダー設定</title>
      </head>
      <body>
        <h1>カレンダー設定</h1>
        本体内部のカレンダーを設定します。<br>
          <br>
          <div style= 'text-align: center;'>
        <span style='font-size: 1.5em;'>%RTC_Y% / %RTC_M% / %RTC_D%<br>
        ↓ ↓ ↓<br></span>
        <form action='/calendar' method='get'>
      
          <input type='date' name='date' min='2025-01-01' max='2090-12-31' style = 'font-size: 1.5em; text-align: center;'>
          </div>
          <br>
          スマホの日付がデフォルト入力されています。<br>
          未来の日付に設定すると動作テストができます。<br>
          <br>
          <input type='time' name='time' readonly tabIndex = '-1' style = 'background-color: gainsboro; cursor:not-allowed;'><br>
          ※時刻はスマホの時計から自動取得されます。<br>
          
          <br>
      
          <input type='submit' value='設定' style='font-size: 1.5em; padding:0.5em; width:100%;'>
        </form>
        <br><br>
        <a href='/'>← ホームへ戻る</a>
      </body>
      <script defer>
       
          function updateTime() {
              const timeInputs = document.getElementsByName("time");
              if (timeInputs.length > 0) {
                  const now = new Date();
                  const hours = String(now.getHours()).padStart(2, '0');
                  const minutes = String(now.getMinutes()).padStart(2, '0');
                  const seconds = String(now.getSeconds()).padStart(2, '0');
                  const formattedTime = `${hours}:${minutes}:${seconds}`;
                  
                  const timeInputs = document.getElementsByName("time");
                  timeInputs[0].value = formattedTime;
              }
          }
      
          function getDate() {
              const dateInputs = document.getElementsByName("date");
              if (dateInputs.length > 0) {
                  const now = new Date();
                  const formattedDate = now.toISOString().split('T')[0];
                  const dateInputs = document.getElementsByName("date");
                  dateInputs[0].value = formattedDate;
              }
          }
      
      
          document.addEventListener("DOMContentLoaded", function () {
              // 初回実行
              getDate();
              updateTime();
              
              // 時計は1秒ごとに更新
              setInterval(updateTime, 1000); 
          });
      </script>
      </html>
    )rawliteral";



#endif