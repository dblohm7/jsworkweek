function result(ok, msg)
{
  postMessage({ok: ok, msg: msg});
}

function JSONTester(iterations, users, resultFn) {
  this.iterations = iterations;
  this.resultFn = resultFn;
  this.json_data = [];
  this.obj_data = [];
  for (var i = 0; i < users.length; ++i) {
    var url = users[i] + ".json";
    var request = new XMLHttpRequest();
    request.open("GET", url, false);
    request.send();
    if (request.status != 200 && request.status != 304) {
      resultFn(false, "Failed loading " + url + ", status is " + request.status);
    } else {
      this.json_data.push(request.responseText);
    }
  }
}

function TestData(dataIndex, fn, outFn) {
  this.dataIndex = dataIndex;
  this.fn = fn;
  this.results = [];
  this.sum = 0;
  this.outFn = outFn;
}

TestData.prototype.outputResults = function() {
  var avg = this.sum / this.results.length;
  var variance = 0;
  this.results.forEach(function(x) {
    var delta = x - avg;
    variance += delta * delta;
  });
  var stddev = Math.sqrt(variance / this.results.length);
  var stringLength = this.fn[this.fn.fieldForLength][this.dataIndex].length;
  var cps = stringLength / (avg * 1000);
  this.outFn(true, this.fn.name + ": String length: " + stringLength + ", Min: "
                   + Math.min.apply(null, this.results) + " ms, Max: " 
                   + Math.max.apply(null, this.results) + " ms, Avg: " + avg 
                   + " ms (" + cps + " million chars per second), Std Dev: " 
                   + stddev);
};

TestData.prototype.appendResult = function(startMs, endMs) {
  var ms = endMs - startMs;
  this.results.push(ms);
  this.sum += ms;
};

JSONTester.prototype.run = function() {
  var self = this;
  var fns = [{name: "Parsing", fieldForLength: "input", input: self.json_data,
              fn: JSON.parse, output: self.obj_data},
             {name: "Serialization", fieldForLength: "output",
              input: self.obj_data, fn: JSON.stringify, output: self.json_data}];
  var data = [];
  this.json_data.forEach(function(x) {
    data.push([]);
    fns.forEach(function(y) {
      data[data.length - 1].push(new TestData(data.length - 1, y, self.resultFn));
    });
  });

  for (var i = 0; i < this.json_data.length; ++i) {
    var start_ms;
    var end_ms;
    for (var j = 0; j < this.iterations; ++j) {
      for (var k = 0; k < fns.length; ++k) {
        start_ms = Date.now();
        fns[k].output[i] = fns[k].fn(fns[k].input[i]);
        end_ms = Date.now();
        data[i][k].appendResult(start_ms, end_ms);
      }
    }
  }
  data.forEach(function(x) {
    x.forEach(function(y) {
      y.outputResults();
    });
  });
  this.resultFn(true, null);
};

function test(iterations)
{
  var tester = new JSONTester(iterations,
                              ["calgaryherald", "tarasglek", "dblohm7"],
                              result);
  tester.run();
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
