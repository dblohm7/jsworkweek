function result(ok, msg)
{
  postMessage({ok: ok, msg: msg});
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
      result(false, "Failed loading " + url + ", status is " + request.status);
      return false;
    }
    json_data[i] = request.responseText;
  }
  var jsobject = [];
  var bytes = 0;
  var index = 0;
  var start_ms;
  var end_ms;

  for (var i = 0; i < json_data.length; ++i) {
    start_ms = Date.now();
    for (var j = 0; j < iterations; ++j) {
      jsobject[i] = JSON.parse(json_data[i]);
      bytes += json_data[i].length;
    }
    end_ms = Date.now();
    var bytesPerSec = bytes / 1000 / (end_ms - start_ms);
    result(true, "Parsed " + bytesPerSec + " million characters per second");

    bytes = 0;
    start_ms = Date.now();
    for (var j = 0; j < iterations; ++j) {
      json_data[i] = JSON.stringify(jsobject[i]);
      bytes += json_data[i].length;
    }
    end_ms = Date.now();
    bytesPerSec = bytes / 1000 / (end_ms - start_ms);
    result(true, "Serialized " + bytesPerSec + " million characters per second");
  }
  result(true, null);
  return true;
}

onmessage = function(event) {
  var data = event.data;
  try {
    test(data.iterations);
  } catch(e) {
    result(false, e.toString());
  }
};
