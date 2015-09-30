Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('https://benkrejci.com/face-plant/config.htm');
});

var RETRIES = 3;
var RETRY_PAUSE = 5 * 1000;
var xhrRequest = function (url, type, callback, retry) {
  retry = retry || 0;
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    try {
      if (!this.responseText) throw new Error('No data!');
      callback(this.responseText);
    } catch (e) {
      if (retry < RETRIES) {
        setTimeout(function () {
          xhrRequest(url, type, callback, retry + 1);
        }, RETRY_PAUSE);
      } else {
        console.error('XMLHttpRequest failed ' + ( retry + 1 ) + ' times: ' + String(e));
      }
    }
  };
  xhr.open(type, url);
  xhr.send();
};

function KtoC(k) {
  return k - 273.15;
}
  
function CtoF(c) {
  return c * 9 / 5 + 32;
}
  
function round(x) {
  return x != null && String(x).replace(/\..*$/,'');
}

var DAYS_ABBR = [
  'Sun',
  'Mon',
  'Tue',
  'Wed',
  'Thu',
  'Fri',
  'Sat'
];
var WEATHER_ICONS;
var TODAYS_FORECAST_HOURS_CUTOFF = 15;
var sunrise, sunset;

var getWeatherIcon = function (weatherInfo, isNight) {
  var date = new Date();
  isNight = isNight == null ?
              ( +date < +sunrise || sunrise.getDay() > date.getDay() ) ||
              ( +date > +sunset  || sunset.getDay()  > date.getDay() )
            : isNight;
  if (!weatherInfo) return '';
  var icon;
  if (isNight) icon = WEATHER_ICONS[weatherInfo.id + '-n'];
  return icon || WEATHER_ICONS[weatherInfo.id];
};

function locationSuccess(pos) {
  var locationArgs = 'lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude;
  var url = 'http://api.openweathermap.org/data/2.5/weather?' + locationArgs;
  xhrRequest(url, 'GET', function (responseText) {
    var json = JSON.parse(responseText);
    sunrise = new Date(json.sys.sunrise * 1000);
    sunset = new Date(json.sys.sunset * 1000);
    
    var tempC = KtoC(json.main.temp);
    var tempF = CtoF(tempC);
    var appData = {
      'KEY_WEATHER_TEMP_C': round(tempC),
      'KEY_WEATHER_TEMP_F': round(tempF),
      'KEY_WEATHER_CONDITIONS': getWeatherIcon(json.weather[0]),
      'KEY_CITY': json.name
    };
    
    Pebble.sendAppMessage(appData, function (e) {
      var url = 'http://api.openweathermap.org/data/2.5/forecast/daily?cnt=5&' + locationArgs;
      xhrRequest(url, 'GET', function (responseText) {
        var json = JSON.parse(responseText);
        var appData = {};
        var date = new Date();
        var day = date.getDay();
        if (date.getHours() >= TODAYS_FORECAST_HOURS_CUTOFF)
          day++;
        day = day % 7;
        while ((new Date(json.list[0].dt * 1000)).getDay() != day) {
          json.list.shift();
        }
        json.list.slice(0, 3).forEach(function (data, i) {
          var minC = KtoC(data.temp.min);
          var minF = CtoF(minC);
          var maxC = KtoC(data.temp.max);
          var maxF = CtoF(maxC);
          appData['KEY_FORECAST_' + i + '_MIN_C'] = round(minC);
          appData['KEY_FORECAST_' + i + '_MAX_C'] = round(maxC);
          appData['KEY_FORECAST_' + i + '_MIN_F'] = round(minF);
          appData['KEY_FORECAST_' + i + '_MAX_F'] = round(maxF);
          appData['KEY_FORECAST_' + i + '_CONDITIONS'] = getWeatherIcon(data.weather[0], false);
          appData['KEY_FORECAST_' + i + '_DAY'] = DAYS_ABBR[(new Date(data.dt * 1000)).getDay()];
        });
        Pebble.sendAppMessage(appData, function (e) {}, function (e) {console.error('error requesting forecast!');});
      });
    }, function (e) {console.error('error requesting current weather!');});
  });
}

function locationError(err) {
  console.log('error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

/*Pebble.addEventListener('ready', 
  function(e) {
    getWeather();
  }
);*/

Pebble.addEventListener('appmessage',
  function(e) {
    getWeather();
  }                     
);

WEATHER_ICONS = {
  "": "?",
/*   Thunderstorm - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
/* thunderstorm with light rain */
  "200": "\uEB28",
  "201": "\uEB29",
  "202": "\uEB2A",
  "210": "\uEB32",
  "211": "\uEB33",
  "212": "\uEB34",
  "221": "\uEB3D",
  "230": "\uEB46",
  "231": "\uEB47",
  "232": "\uEB48",
/*   Drizzle - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
/*  light intensity drizzle */
  "300": "\uEB8C",
  "301": "\uEB8D",
  "302": "\uEB8E",
  "310": "\uEB96",
  "311": "\uEB97",
  "312": "\uEB98",
  "313": "\uEB99",
  "314": "\uEB9A",
  "321": "\uEBA1",
/*   Rain - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
/* light rain  */
  "500": "\uEC54",
  "501": "\uEC55",
  "502": "\uEC56",
  "503": "\uEC57",
  "504": "\uEC58",
  "511": "\uEC5F",
  "520": "\uEC68",
  "521": "\uEC69",
  "522": "\uEC6A",
  "531": "\uEC73",
/*   Snow - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
/* light snow  */
  "600": "\uECB8",
  "601": "\uECB9",
  "602": "\uECBA",
  "611": "\uECC3",
  "612": "\uECC4",
  "615": "\uECC7",
  "616": "\uECC8",
  "620": "\uECCC",
  "621": "\uECCD",
  "622": "\uECCE",
/*   Atmosphere - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
/* mist */
  "701": "\uED1D",
  "711": "\uED27",
  "721": "\uED31",
  "731": "\uED3B",
  "741": "\uED45",
  "751": "\uED4F",
  "761": "\uED59",
  "762": "\uED5A",
  "771": "\uED63",
  "781": "\uED6D",
/*   Clouds - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
/*  sky is clear  */  /*  Calm  */
  "800": "\uED80",
  "951": "\uED80",
  "800-n": "\uF168",
  "951-n": "\uF168",
/*  few clouds   */
  "801": "\uED81",
  "801-n": "\uF169",
  "802": "\uED82",
  "802-n": "\uF16A",
/* broken clouds  */
  "803": "\uED83",
  "804": "\uED84",
/*   Extreme - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
/* tornado  */
  "900": "\uEDE4",
  "901": "\uEDE5",
  "902": "\uEDE6",
  "903": "\uEDE7",
  "904": "\uEDE8",
  "905": "\uEDE9",
  "906": "\uEDEA",
/*   Additional - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
/* Setting */
  "950": "\uEE16",
  "952": "\uEE18",
  "953": "\uEE19",
  "954": "\uEE1A",
  "955": "\uEE1B",
  "956": "\uEE1C",
  "957": "\uEE1D",
  "958": "\uEE1E",
  "959": "\uEE1F",
  "960": "\uEE20",
  "961": "\uEE21",
  "962": "\uEE22"
};