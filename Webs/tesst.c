string str="<!DOCTYPE html>\\
<html>\\
  <head>\
    <meta charset=\"utf-8\">\
    <title>Submit and Clear</title>\
  </head>\
  <body>\
  <div style=\"text-align: center;\">\
    <form >\
      <input type=\"text\" name=\"userInput\">\
      <input type=\"submit\" value=\"Submit\">\
      <button type=\"button\" onclick=\"clearInput()\">Clear</button>\
    </form>\
	 <textarea rows=\"10\" cols=\"50\" readonly></textarea>\
	</div>\
  </body>\
</html>\
<style>\
  input[type=\"text\"] {\
    width: 200px;\
    height: 30px;\
    background-color: #cccccc;\
    margin: 0 auto;\
  }\
  input[type=\"submit\"] {\
    width: 100px;\
    height: 30px;\
    background-color: #4CAF50;\
  }\
  button {\
    width: 100px;\
    height: 30px;\
    background-color: #FF5722;\
  }\
  .center {\
  margin: 0 auto;\
  }\
}\
</style>\
\
\
<script>\
  // 清除文本框的内容\
  function clearInput() {\
    document.querySelector(\"input[type='text']\").value = \"\";\
\
    // 创建 XMLHttpRequest 对象\
    var xhr = new XMLHttpRequest();\
\
    // 设置请求方法和请求地址\
    xhr.open(\"POST\", \"/submit\");\
\
    // 设置发送的数据的格式\
    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\
\
    // 发送请求\
    xhr.send(\"clear\");\
  }\
\
  document.querySelector(\"input[type='submit']\").addEventListener(\"click\", function(event) {\
    // 阻止表单的默认提交行为\
    event.preventDefault();\
\
    // 获取文本框的内容\
    var userInput = document.querySelector(\"input[type='text']\").value;\
\
    // 创建 XMLHttpRequest 对象\
    var xhr = new XMLHttpRequest();\
\
    // 设置请求方法和请求地址\
    xhr.open(\"POST\", \"/submit\");\
\
    // 设置发送的数据的格式\
    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\
\
    // 发送请求\
    xhr.send(encodeURIComponent(userInput));\
\
    // 添加文本框内的内容到记事本中\
    document.querySelector(\"textarea\").value += userInput + \"\n\";\
  });\
</script>";



old="<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\" />\
    <title>表单标签</title>\
  </head>\
  <body>\
    <form action=\"/echo\" method=\"post\">\
      事项：<input name=\"words\" /><br />\
      <!--登陆按钮-->\
      <input type=\"submit\" value=\"登录\" />\
    </form>\
    <form action=\"/echo\" method=\"post\">\
        <input type=\"submit\" value=\"清空\" />\
      </form>\
  </body>\
</html>";