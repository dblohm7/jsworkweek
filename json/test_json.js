function result(ok, index, msg)
{
  postMessage({ok: ok, index: index, msg: msg});
}

function test(iterations)
{
  var json_data = [];
  var twitter_users = ["calgaryherald", "tarasglek", "dblohm7"];
  for (var i = 0; i < twitter_users.length; ++i) {
    var url = twitter_users[i] + ".json";
    var request = new XMLHttpRequest();
    request.open("GET", url, false);
    request.send();
    if (request.status != 200 && request.status != 304) {
      result(false, 0, "Failed loading " + url + ", status is " + request.status);
      return false;
    }
    json_data[i] = request.responseText;
  }
  var jsobject = [];
  var bytes = 0;
  var index = 0;
  var start_ms;
  var end_ms;
  var j;
  var k;

  start_ms = Date.now();
  for (j = 0; j < iterations; ++j) {
    index = j % json_data.length;
    jsobject[index] = JSON.parse(json_data[index]);
    bytes += json_data[index].length;
  }
  end_ms = Date.now();
  var bytesPerSec = bytes / 1000 / (end_ms - start_ms);
  result(true, 0, "Parsed " + bytesPerSec + " million characters per second");

  bytes = 0;
  start_ms = Date.now();
  for (k = 0; k < iterations; ++k) {
    index = k % jsobject.length;
    json_data[index] = JSON.stringify(jsobject[index]);
    bytes += json_data[index].length;
  }
  end_ms = Date.now();
  bytesPerSec = bytes / 1000 / (end_ms - start_ms);
  result(true, 1, "Serialized " + bytesPerSec + " million characters per second");
  result(true, 2);
  return true;
}

onmessage = function(event) {
  var data = event.data;
  try {
    test(data.iterations);
  } catch(e) {
    result(false, 0, e.toString());
  }
};
