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
        点滅回数: <input type='number' name='count' min='1' max='20'>
        <input type='submit' value='設定'>
      </form>
      <br>
      <a href='/'>← ホームへ戻る</a>
    </body>
    </html>
    )rawliteral";

#endif