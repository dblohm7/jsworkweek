Components.utils.import("resource://gre/modules/ctypes.jsm");

const ITERATIONS = 10000000;
const CHROME_SCRIPT_TIMEOUT = "dom.max_chrome_script_run_time";

function test()
{
  var results_div = document.getElementById("results");
  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
  var prev_script_timeout = prefs.getIntPref(CHROME_SCRIPT_TIMEOUT);
  prefs.setIntPref(CHROME_SCRIPT_TIMEOUT, 0);
  var libc;
  try {
    libc = ctypes.open("libc.so.6");
  } catch(e) {
    libc = ctypes.open("libc.so");
  }

  var char_array = ctypes.ArrayType(ctypes.char);
  // struct stat on 32-bit ubuntu lucid
  var struct_stat = new char_array(88);

  var fxstat;
  try {
    fxstat = libc.declare("__fxstat", ctypes.default_abi, ctypes.int, ctypes.int,
			 ctypes.int, ctypes.void_t.ptr);
  } catch(e) {
    alert(e.toString());
  }

  var start_time;
  var end_time;
  var min = 0xFFFFFFFF;
  var max = 0;
  var ns;
  var sum = 0;
  var variance = 0;
  var results = [];
  for (var i = 0; i < ITERATIONS; ++i) {
    start_time = window.performance.now();
    fxstat(3, 0, struct_stat);
    end_time = window.performance.now();
    ns = (end_time - start_time) * 1000000;
    min = Math.min(min, ns);
    max = Math.max(max, ns);
    sum += ns;
    results.push(ns);
  }
  avg = sum / results.length;
  results.forEach(function(x) {
    var delta = x - avg;
    variance += delta * delta;
  });
  var sd = Math.sqrt(variance / results.length);
  // alert("Ran " + ITERATIONS + " iterations in " + (end_time - start_time) + " milliseconds");
  // results_div.textContent = "Last test ran " + ITERATIONS + " iterations in " + (end_time - start_time) + " milliseconds";
  results_div.textContent = "Min: " + min + " ns, Max: " + max + " ns, Avg: " + avg + " ns, Std Dev: " + sd;
  prefs.setIntPref(CHROME_SCRIPT_TIMEOUT, prev_script_timeout);
  return true;
}

function init()
{
  var btn = document.getElementById("start");
  btn.onclick = test;
}

