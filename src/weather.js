var TODAYS_FORECAST_HOURS_CUTOFF = 15; // 3:00 PM
var CONFIG_PAGE_URL = 'http://benkrejci.com/fourcast/0.8/config.html';
var OPENWEATHERMAP_API_KEY = '746ca9a1c69c304bec844202dd4a501e';

var configDict;

Pebble.addEventListener('ready', function (e) {
  console.debug('listener:ready');
  console.debug('sendAppMessage:ready');
  Pebble.sendAppMessage({ KEY_READY: 1 }, function (e) {
    console.debug('sendAppMessage:ready:success');
  }, function (e) {
    console.error('sendAppMessage:ready:error: ' + String(e));
  });
});

Pebble.addEventListener('showConfiguration', function(e) {
  console.debug('listener:showConfiguration');
  var configJson = localStorage.config;
  Pebble.openURL(CONFIG_PAGE_URL + '#' + encodeURIComponent(configJson));
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.debug('listener:webviewclosed');
  if (!e.response) return;
  var configJson;
  try {
    configJson = decodeURIComponent(e.response);
  } catch (error) {
    return;
  }
  if ( !configJson || configJson == '{}' ||
       configJson == localStorage.config ) return;
  try {
    configDict = JSON.parse(configJson);
  } catch (error) {
    return;
  }
  localStorage.config = configJson;
  sendConfigData();
});

Pebble.addEventListener('appmessage', function (e) {
  console.debug('listener:appmessage');
  if (e.payload.KEY_ACTION_STORE_SETTINGS) {
    configDict = {
      supports_color: !!e.payload.KEY_SUPPORTS_COLOR,
      temp_units: e.payload.KEY_TEMP_UNITS,
      bg_color: gColorToHex(e.payload.KEY_BG_COLOR),
      text_color: gColorToHex(e.payload.KEY_TEXT_COLOR),
      weather_service: e.payload.KEY_WEATHER_SERVICE,
      weather_underground_api_key: e.payload.KEY_WEATHER_UNDERGROUND_API_KEY
    };
    localStorage.config = JSON.stringify(configDict);
    console.debug('action:store_settings: ' + localStorage.config);
  }
  
  if (e.payload.KEY_ACTION_GET_WEATHER) {
    console.debug('action:get_weather');
    getWeather();
  }
});

function sendConfigData() {
  console.debug('sendConfigData()');
  var configJson = localStorage.config;
  if (!configJson) return;
  var config = JSON.parse(configJson);
  var dict = {
    KEY_TEMP_UNITS: config.temp_units,
    KEY_BG_COLOR: hexToGColor(config.bg_color),
    KEY_TEXT_COLOR: hexToGColor(config.text_color),
    KEY_WEATHER_SERVICE: config.weather_service,
    KEY_WEATHER_UNDERGROUND_API_KEY: config.weather_underground_api_key
  };
  console.debug('sendAppMessage:configData: ' + JSON.stringify(dict));
  Pebble.sendAppMessage(dict, function (e) {
    console.debug('sendAppMessage:configData:success');
    getWeather();
  }, function (e) {
    console.error('sendAppMessage:configData:error: ' + String(e));
  });
}

function getWeather() {
  console.debug('getWeather()');
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    function (e) {
      console.error('getCurrentPosition:error: ' + String(e));
    },
    { enableHighAccuracy: false,
      timeout: 60 * 1000,
      maximumAge: 15 * 60 * 1000 }
  );
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

function locationSuccess(pos) {
  console.debug('locationSuccess([' + pos.coords.latitude + ',' + pos.coords.longitude + '])');
  switch (configDict.weather_service) {
    case 'OpenWeatherMap':
      getOpenWeatherMap(pos);
    break;
    case 'Weather Underground':
      getWeatherUnderground(pos);
    break;
  }
}

function getOpenWeatherMap(pos) {
  var locationArgs = 'APPID=' + OPENWEATHERMAP_API_KEY +
                     '&lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude;
  var url = 'http://api.openweathermap.org/data/2.5/weather?' + locationArgs;
  xhr(url, 'GET', function (responseText) {
    var json = JSON.parse(responseText);
    
    var sunrise = new Date(json.sys.sunrise * 1000);
    var sunset = new Date(json.sys.sunset * 1000);
    var date = new Date();
    var dateMinutes = date.getHours() * 60 + date.getMinutes();
    var isNight = dateMinutes < ( sunrise.getHours() * 60 + sunrise.getMinutes() ) ||
                  dateMinutes >= ( sunset.getHours() * 60 + sunset.getMinutes() );
    
    var tempC = KtoC(json.main.temp);
    var tempF = CtoF(tempC);
    
    var appData = {
      'KEY_WEATHER_TEMP_C': round(tempC),
      'KEY_WEATHER_TEMP_F': round(tempF),
      'KEY_WEATHER_CONDITIONS': getOpenWeatherMapIcon(json.weather[0] && json.weather[0].id, isNight),
      'KEY_CITY': json.name
    };
    
    console.debug('sendAppMessage:weather: ' + JSON.stringify(appData));
    Pebble.sendAppMessage(appData, function (e) {
      console.debug('sendAppMessage:weather:success');
      var url = 'http://api.openweathermap.org/data/2.5/forecast/daily?cnt=5&' + locationArgs;
      xhr(url, 'GET', function (responseText) {
        var json = JSON.parse(responseText);
        var appData = {};
        var date = new Date();
        var day = date.getDay();
        if (date.getHours() >= TODAYS_FORECAST_HOURS_CUTOFF)
          day++;
        day = day % 7;
        while ((new Date(json.list[0].dt * 1000)).getDay() != day && json.list.length) {
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
          appData['KEY_FORECAST_' + i + '_CONDITIONS'] = getOpenWeatherMapIcon(data.weather[0] && data.weather[0].id, false);
          appData['KEY_FORECAST_' + i + '_DAY'] = DAYS_ABBR[(new Date(data.dt * 1000)).getDay()];
        });
        
        console.debug('sendAppMessage:forecast:' + JSON.stringify(appData));
        Pebble.sendAppMessage(appData, function (e) {
          console.debug('sendAppMessage:forecast:success');
        }, function (e) {
          console.error('sendAppMessage:forecast:error: ' + e);
        });
      });
    }, function (e) {
      console.error('sendAppMessage:weather:error: ' + e);
    });
  });
}

function getWeatherUnderground(pos) {
  var url = 'http://api.wunderground.com/api/' +
            configDict.weather_underground_api_key + '/' +
            'astronomy/conditions/forecast/' +
            'q/' + pos.coords.latitude + ',' + pos.coords.longitude +
            '.json';
  xhr(url, 'GET', function (responseText) {
    var json = JSON.parse(responseText);
    
    var localTime = json.current_observation.local_time_rfc822.match(/(\d{1,2}):(\d{1,2}):\d{1,2}/);
    localTime = localTime && { hour: localTime[1], minute: localTime[2] };
    var localMinutes = localTime && ( parseInt(localTime.hour) * 60 + parseInt(localTime.minute) );
    var isNight = !isNaN(localTime) &&
                  ( localMinutes < ( json.sun_phase.sunrise.hour * 60 + json.sun_phase.sunrise.minute ) ||
                    localMinutes >= ( json.sun_phase.sunset.hour * 60 + json.sun_phase.sunset.minute ) );
    
    var appData = {
      'KEY_WEATHER_TEMP_C': round(json.current_observation.temp_c),
      'KEY_WEATHER_TEMP_F': round(json.current_observation.temp_f),
      'KEY_WEATHER_CONDITIONS': getWeatherUndergroundIcon(json.current_observation.icon, isNight),
      'KEY_CITY': json.current_observation.display_location.city
    };
    
    var date = new Date();
    var day = date.getDay();
    if (date.getHours() >= TODAYS_FORECAST_HOURS_CUTOFF)
      day++;
    day = day % 7;
    var forecastday = json.forecast.simpleforecast.forecastday;
    while ((new Date(parseInt(forecastday[0].date.epoch) * 1000)).getDay() != day && forecastday.length) {
      forecastday.shift();
    }
    forecastday.slice(0, 3).forEach(function (data, i) {
      appData['KEY_FORECAST_' + i + '_MIN_C'] = round(data.low.celsius);
      appData['KEY_FORECAST_' + i + '_MAX_C'] = round(data.high.celsius);
      appData['KEY_FORECAST_' + i + '_MIN_F'] = round(data.low.fahrenheit);
      appData['KEY_FORECAST_' + i + '_MAX_F'] = round(data.high.fahrenheit);
      appData['KEY_FORECAST_' + i + '_CONDITIONS'] = getWeatherUndergroundIcon(data.icon, false);
      appData['KEY_FORECAST_' + i + '_DAY'] = DAYS_ABBR[(new Date(parseInt(data.date.epoch) * 1000)).getDay()];
    });

    console.debug('sendAppMessage:weather:' + JSON.stringify(appData));
    Pebble.sendAppMessage(appData, function (e) {
      console.debug('sendAppMessage:weather:success');
    }, function (e) {
      console.error('sendAppMessage:weather:error: ' + e);
    });
  });
}

var RETRIES = 3;
var RETRY_PAUSE = 5 * 1000;
function xhr(url, type, callback, retry) {
  console.debug('xhr:start:' + url);
  retry = retry || 0;
  var _xhr = new XMLHttpRequest();
  _xhr.onload = function () {
    try {
      if (!this.responseText) throw new Error('No data!');
      console.debug('xhr:success: ' + this.responseText.slice(0,32) + '...');
      callback(this.responseText);
    } catch (e) {
      if (retry < RETRIES) {
        console.error('xhr:fail: ' + ( retry + 1 ) + ' times, trying again (' + String(e) + ')');
        setTimeout(function () {
          xhr(url, type, callback, retry + 1);
        }, RETRY_PAUSE);
      } else {
        console.error('xhr:fail: ' + ( retry + 1 ) + ' times: ' + String(e));
      }
    }
  };
  _xhr.open(type, url);
  _xhr.send();
}

function KtoC(k) {
  return k - 273.15;
}
  
function CtoF(c) {
  return c * 9 / 5 + 32;
}
  
function round(x) {
  return x != null && String(x).replace(/\..*$/,'');
}

function hexToGColor(hex) {
  var hexNum = parseInt(hex, 16);
  return (                0xFF >> 6 << 6 ) + // a
         ( hexNum >> 16 & 0xFF >> 6 << 4 ) + // r
         ( hexNum >>  8 & 0xFF >> 6 << 2 ) + // g
         ( hexNum       & 0xFF >> 6      );  // b
}

function gColorToHex(gColor) {
  return ( '000000' +
           ( ( ( gColor >> 4 & 3 ) * 0xff / 3 << 16 ) +
             ( ( gColor >> 2 & 3 ) * 0xff / 3 << 8  ) +
             ( ( gColor >> 0 & 3 ) * 0xff / 3       ) )
             .toString(16) )
           .slice(-6);
}

function getOpenWeatherMapIcon(iconId, isNight) {
  return isNight && OPEN_WEATHER_MAP_ICONS[iconId + '-n'] || OPEN_WEATHER_MAP_ICONS[iconId];
}

var OPEN_WEATHER_MAP_ICONS = {
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

function getWeatherUndergroundIcon(iconName, isNight) {
  var altIconName = iconName.replace(/^chance/, '');
  altIconName = altIconName.length != iconName.length && altIconName;
  var icon = isNight &&
         ( WEATHER_UNDERGROUND_ICONS[iconName + '-n'] ||
           ( altIconName &&
             WEATHER_UNDERGROUND_ICONS[altIconName + '-n'] ) ) ||
         WEATHER_UNDERGROUND_ICONS[iconName] ||
           ( altIconName &&
             WEATHER_UNDERGROUND_ICONS[altIconName] );
  if (!icon) console.error('getWeatherUndergroundIcon(' + iconName + ',' + isNight + '):error:could not find icon!');
  return icon;
}

var WEATHER_UNDERGROUND_ICONS = {
  "": "?",
/*   Thunderstorm - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
  "chancetstorms": "\uEB32",
  "tstorms": "\uEB33",
/*   Rain - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
  "chancerain": "\uEC54",
  "rain": "\uEC56",
/*   Snow - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
  "flurries": "\uECB8",
  "chancesnow": "\uECB9",
  "snow": "\uECBA",
  "sleet": "\uECC3",
/*   Atmosphere - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
  "hazy": "\uED31",
  "fog": "\uED45",
/*   Clouds - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    */
  "clear": "\uED80",
  "sunny": "\uED80",
  "clear-n": "\uF168",
  "sunny-n": "\uF168",
  "mostlysunny": "\uED81",
  "mostlysunny-n": "\uF169",
  "partlycloudy": "\uED81",
  "partlycloudy-n": "\uF169",
  "mostlycloudy": "\uED82",
  "mostlycloudy-n": "\uF16A",
  "partlysunny": "\uED82",
  "partlysunny-n": "\uF16A",
  "cloudy": "\uED84"
};