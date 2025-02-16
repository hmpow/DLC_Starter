#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const String SETTING_HTML = R"rawliteral(
<!DOCTYPE html>
<html>
<meta lang='ja'>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>LED点滅設定</title>
</head>
<body>
  <h1>LED点滅設定 外部ファイルから</h1>
  <form action='/' method='get'>
    点滅回数: <input type='number' name='count' min='1' max='20'>
    <br>
    <input type='submit' value='設定'>
  </form>
</body>
</html>
)rawliteral";

#endif