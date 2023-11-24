char index_html[] ="\
<!DOCTYPE html>\
<html>\
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
		<br />\
	 <textarea rows=\"10\" cols=\"50\" readonly></textarea>\
	</div>\
	<br />\
	<div style=\"text-align: center;\">\
	<input type=\"file\" id=\"input-image\" onchange=\"selectImage()\" />\
    <button onclick=\"postImage()\">Post Image</button>\
    <br />\
    <canvas id=\"canvas\" style=\"display: none;\"></canvas>\
    <img id=\"image\" width=\"400\" height=\"300\" />\
	</div>\
	<div div style=\"text-align: center;\">\
      <p>阈值: <span id=\"grayValue\">128</span></p>\
      <input type=\"range\" min=\"0\" max=\"255\" value=\"128\" oninput=\"updateValue(this.value)\" />\
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
\
</style>\
<script>\
  let Gray_value = 128;\
      function updateValue(value) {\
        Gray_value = value;\
        document.getElementById(\"grayValue\").innerHTML = Gray_value;\
		selectImage();\
      }\
	  \
  function clearInput() {\
    document.querySelector(\"input[type='text']\").value = \"\";\
    var xhr = new XMLHttpRequest();\
    xhr.open(\"POST\", \"/submit\");\
    xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\
    xhr.send(encodeURIComponent(\"clear\"));\
  }\
  document.querySelector(\"input[type='submit']\").addEventListener(\"click\", function(event) {\
        event.preventDefault();\
        var userInput = document.querySelector(\"input[type='text']\").value;\
        var xhr = new XMLHttpRequest();\
    xhr.open(\"POST\", \"/submit\");\
        xhr.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\
            var ret =false;\
    for(var i=0;i<userInput.length;i++)\
      ret=(userInput.charCodeAt(i)>=10000)||ret  ;\
      if(ret){\
        xhr.send(userInput);\
      }else{\
      console.log(\"ssss\");\
      xhr.send(encodeURIComponent(userInput));\
    }\
        document.querySelector(\"textarea\").value += userInput + \";\";\
  });\
  \
  var imgArray;\
      function selectImage() {\
                var input = document.getElementById('input-image');\
        var file = input.files[0];\
        var reader = new FileReader();\
                reader.onload = function(e) {\
          var img = document.getElementById('image');\
          img.onload = function() {\
                        var canvas = document.getElementById('canvas');\
            var ctx = canvas.getContext('2d');\
            var imgWidth = img.width;\
            var imgHeight = img.height;\
            canvas.width = imgWidth;\
            canvas.height = imgHeight;\
            ctx.drawImage(img, 0, 0, imgWidth, imgHeight);\
            canvas.willReadFrequently = true;\
            var imgData = ctx.getImageData(0, 0, imgWidth, imgHeight);\
			\
			var idataTrg=floyd_steinberg(imgData,Gray_value);\
			\
			imgArray = [];\
			\
			for (var i = 0; i < idataTrg.data.length; i += 4) {\
              var avg = (idataTrg.data[i] + idataTrg.data[i + 1] + idataTrg.data[i + 2]) / 3;\
              imgArray.push(avg < 255 ? 1 : 0);\
			}\
			ctx.putImageData(idataTrg, 0, 0);\
            img.src = canvas.toDataURL();\
          };\
		  \
          img.src = e.target.result;\
        };\
        reader.readAsDataURL(file);\
      }\
function floyd_steinberg(image,pare_value) {\
  var imageData = image.data;\
  var imageDataLength = imageData.length;\
  var w = image.width;\
  var lumR = [],\
      lumG = [],\
      lumB = [];\
  var newPixel, err;\
  for (var i = 0; i < 256; i++) {\
    lumR[i] = i * 0.299;\
    lumG[i] = i * 0.587;\
    lumB[i] = i * 0.110;\
  }\
    for (var i = 0; i <= imageDataLength; i += 4) {\
    imageData[i] = Math.floor(lumR[imageData[i]] + lumG[imageData[i+1]] + lumB[imageData[i+2]]);\
  }\
  for (var currentPixel = 0; currentPixel <= imageDataLength; currentPixel += 4) {\
        newPixel = imageData[currentPixel] < Gray_value ? 0 : 255;\
    err = Math.floor((imageData[currentPixel] - newPixel) / 23);\
    imageData[currentPixel + 0 * 1 - 0 ] = newPixel;\
    imageData[currentPixel + 4 * 1 - 0 ] += err * 7;\
    imageData[currentPixel + 4 * w - 4 ] += err * 3;\
    imageData[currentPixel + 4 * w - 0 ] += err * 5;\
    imageData[currentPixel + 4 * w + 4 ] += err * 1;\
        imageData[currentPixel + 1] = imageData[currentPixel + 2] = imageData[currentPixel];\
  }\
  return image;\
}\
function postImage() {\
    \
var hexArray = [];\
for (var i = 0; i < imgArray.length; i += 4) {\
    var hex = parseInt(imgArray.slice(i, i + 4).join(''), 2).toString(16);\
    hexArray.push(hex);\
}\
var imgString = hexArray.join('');\
  console.log(imgString);\
    var i = 0;\
  var intervalId = setInterval(function() {\
        var xhr = new XMLHttpRequest();\
        xhr.open('POST', '/submit');\
        xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');\
        xhr.send('image=' + encodeURIComponent(imgString.slice(i, i + 2000)));\
    i += 2000;\
    if (i >= imgString.length) {\
      clearInterval(intervalId);\
    }\
  }, 100);\
}\
</script>\
";