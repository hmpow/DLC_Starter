#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

//CSS
const char CSS_SETTING_PAGE[] PROGMEM = R"rawliteral(
  :root {
    --sub-color: #000066;
    --hover-color: #0000ff
  }
  
  body{
    width: 100%;
    max-width: 480px;
    margin: auto;
  }
  
  .header_base{
    width: 100%;
    height: 48px;
    max-width: 480px;
    background-color: var(--sub-color);
    text-align: left;
    position: fixed;
    z-index: 1;
    padding: 0;
    vertical-align: middle;
    display: flex;
    flex-direction: row;
  }
  
  .triangle_base{
    width: 48px;
    height: 48px;
    padding: 0;
    float: right;
  }
  
  .triangle {
    width: 48px;
    height: 48px;
    clip-path: polygon(8px 24px, 40px 8px, 40px 40px);
    text-align: center;
    padding-left: 6px;
    line-height: 48px;
    background-color: #ffffff;
    font-size: 16px;
  }
  
  .header_title{
    height: 100%;
    padding-left: 8px;
    line-height: 48px;
    color: #ffffff;
    font-weight: bold;
    font-size: 24px;
  }
  
  main
  {
    position: relative;
    top: 48px;
    width: 100%;
    z-index: 0;
    overflow: auto;
    margin: auto 2%;
    width:96%;
  }
  
  input[type="date"],
  input[type="time"] {
    text-align: center;
    text-align-last: center;
    padding-left: 0;
    padding-right: 0;
  }
  
  .input_parts{
    box-sizing: border-box;
    margin-left: 0.1em;
    background-color: #ffffff;
    font-size: 1.2em;
    width: 9em;
    height: 1.8em;
    border-radius: 8px;
    border: 1.5px solid var(--sub-color);
    text-align: center;
  }
    
  .submit_button{
    box-sizing: border-box;
    line-height: 48px;
    height: 48px;
    font-size: 1.5em;
    width: 98%;
    margin: 8px 1%;
    border-radius: 24px;
    background-color: var(--sub-color);
    color: #ffffff;
    text-align: center;
  }
  
  .submit_button:hover {
    box-shadow: 0px 0px 4px var(--hover-color);
  }
  
  .menu_button{
    box-sizing: border-box;
    line-height: 48px;
    height: 48px;
    font-size: 1.5em;
    width: 98%;
    margin: 16px 1%;
    border-radius: 24px;
    background-color: #ffffff;
    color: var(--sub-color);
    border: 2px solid var(--sub-color);
    text-align:center;
  }
  
  .menu_button:hover {
    box-shadow: 0px 0px 4px var(--hover-color);
  }
  
  h3{
    color:var(--sub-color);
    font-size: 1.2em;
  }
  
  em{
    color: var(--sub-color);
    font-weight: bold;
    font-style: normal;
    font-size: 1.2em;
  }
  
  a{
    text-decoration: none;
  }
)rawliteral";


// ホームページ
const char HTML_HOME[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html style='max-width: 480px; margin:auto;'>
  <meta lang='ja'>
  <head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>ホーム</title>
    <style>
      %CSS%
    </style>
  </head>
  <body>
    <header>
	    <div class = "header_base">
        <div class = "header_title">
          設定メニュー
        </div>
      </div>
    </header>
    <main>
	    <a href='/pinsetting'>
	      <div class='menu_button'>マイナ免許 暗証番号登録</div>
	    </a>
        <a href='/calendar'>
	      <div class='menu_button'>カレンダー設定</div>
	    </a>
	    <a href='/endsetting'>
	      <div class='submit_button'>設定を終了し再起動</div>
	    </a>
    </main>
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
    <style>
      %CSS%
    </style>
  </head>
  <body>
    <header>
	    <div class = "header_base">
        <div class = "header_title">
          設定完了
        </div>
      </div>
    </header>
    <main>
      <p>
        本体を再起動します。<br>
        ブラウザを閉じて終了してください。
      </p>
    </main>
  </body>
  </html>
)rawliteral";

// 暗証番号設定ページ
const char HTML_PIN_SETTING[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <meta lang='ja'>
  <head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>マイナ免許 暗証番号登録</title>
    <style>
      %CSS%
    </style>
  </head>
  <body>
    <header>
      <div class = "header_base">
        <div class = "triangle_base">
          <a href='/'>
            <div class = "triangle">
              戻
            </div>
          </a>
        </div>
        <div class = "header_title">
          マイナ免許 暗証番号登録
        </div>
      </div>
    </header>

    <main>
      <form action='/pinsetting' method='get'>
        <p style = 'color:red; font-weight:bold; font-size:1.2em;'>%MESSAGE%</p>
        <b>下記は事前登録不要で使用できます。</b><br>
        ・従来型免許<br>
        ・暗証番号未設定で発行したマイナ免許<br>
        <span style = 'font-size: 0.8em;'>
        詳細は「運転免許証及び運転免許証作成システム等仕様書 バージョン10」をご覧ください。
        「免許証 仕様書」でネット検索するとヒットします。<br>
        </span>
        
        <h3>設定対象ドライバー</h3>
  
        <select name="driver" class = "input_parts">
          <option value = '1'>ドライバー 1</option>
          <option value = '2'>ドライバー 2</option>
          <option value = '3'>ドライバー 3</option>
        </select>
    
        <h3>マイナ免許証暗証番号</h3>
        <input type='number' name='dlcpin' min='0' max='9999' class = 'input_parts'><br>
        ※空白のまま送信すると削除できます。
        <h3>いたずら防止用暗証番号</h3>
        <input type='number' name='secno' min='0' max='9999' class = 'input_parts'><br>
        ※arduino_secrets.h に設定した "SECURITY_NO" です。
        <br>
        <input type='submit' value='設定' class='submit_button'>
      </form>
    </main>
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
    <style>
      %CSS%
    </style>
  </head>
  <body>
    <header>
      <div class = "header_base">
        <div class = "triangle_base">
          <a href='/'>
            <div class = "triangle">
              戻
            </div>
          </a>
        </div>
          <div class = "header_title">
            カレンダー設定
          </div>
        </div>
    </header>


  <main>
    <p>本体内部のカレンダーを設定します。</p>
  
    <div style= 'text-align: center;'>
      <em>%RTC_Y% / %RTC_M% / %RTC_D%<br>
      ↓ ↓ ↓</em>
    <form action='/calendar' method='get'>
      <input type='date' name='date' min='2025-01-01' max='2090-12-31' class = 'input_parts'>
    </div>
      <br>
      スマホの日付がデフォルト入力されています。<br>
      未来の日付に設定すると動作テストができます。<br>
      <br>
      <input type='time' name='time' readonly tabIndex = '-1' style = 'background-color: #ccc; cursor:not-allowed; box-sizing: content-box; width: 8em; border:solid 1px #666; border-radius: 4px;'>
      <br>
      ※時刻はスマホの時計から自動取得されます。<br>

      <input type='submit' value='設定' class='submit_button'>
    </form>
  </main>
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