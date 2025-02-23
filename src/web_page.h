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
        <li><a href='/led'>マイナ免許 暗証番号登録</a></li>
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

    // 暗証番号設定ページ
    const char HTML_LED[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html style='max-width: 480px; margin:auto;'>
    <meta lang='ja'>
    <head>
      <meta charset='utf-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1'>
      <title>マイナ免許 暗証番号登録</title>
    </head>
    <body>
      <h1>マイナ免許 暗証番号登録</h1>
      <form action='/led' method='get'>
        <p>%MESSAGE%</p>
        <b>下記は事前登録不要で使用できます。<br><br>
        ・従来型免許<br>
        ・暗証番号未設定で発行したマイナ免許</b><br><br>
        <span style = 'font-size: 0.8em;'>
        詳細は「運転免許証及び運転免許証作成システム等仕様書 バージョン10」をご覧ください。<br>
        「免許証 仕様書」でネット検索するとヒットします。<br>
        </span>
        
          <h3>設定対象ドライバー</h3>

        <select name="driver" style = 'font-size: 1.2em;'>
          <option value = '1'>ドライバー 1</option>
          <option value = '2'>ドライバー 2</option>
          <option value = '3'>ドライバー 3</option>
        </select>
      
        <h3>マイナ免許証暗証番号</h3>
        <input type='number' name='dlcpin' min='0' max='9999' style = 'font-size: 1.5em;'><br>
          ※空白のまま送信すると削除できます。
        <h3>いたずら防止用暗証番号</h3>
        <input type='number' name='secno' min='0' max='9999' style = 'font-size: 1.5em;'><br>
        ※arduino_secrets.h に設定した "SECURITY_NO" です。
        <br>
        <br>
        <input type='submit' value='設定' style='font-size: 1.5em; padding:0.5em; width:100%; max-width:20em;'>
      </form>
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