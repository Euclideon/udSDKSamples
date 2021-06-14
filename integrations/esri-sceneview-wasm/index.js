const express = require('express')
const request = require('request');
const GtfsRealtimeBindings = require('gtfs-realtime-bindings');
const app = express();
const port = 4006;

app.use(function(req, res, next) {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  res.header("Cross-Origin-Embedder-Policy", "require-corp");
  res.header("Cross-Origin-Opener-Policy", "same-origin");
  next();
});

// Functions
app.get('/', (req, res) => {
  res.writeHead(200, {
    'Content-Type': 'text/plain'
  });
  res.write('Hello World! This is response from the server!\r\nPlease try following commands:\n[host_url]/gtfsrt-all\n[host_url]/gtfsrt-veh-pos\n[host_url]/gtfsrt-trip-update\n[host_url]/gtfsrt-alerts');
  res.end();
});

app.get('/gtfsrt-all', (req, res) => {
  var requestSettings = {
    method: 'GET',
    url: 'https://gtfsrt.api.translink.com.au/api/realtime/SEQ/',
    encoding: null
  };
  request(requestSettings, function (error, response, body) {
    if (!error && response.statusCode == 200) {
      var feed = GtfsRealtimeBindings.transit_realtime.FeedMessage.decode(body);
      res.send(feed.entity);
    }
  });
});

app.get('/gtfsrt-veh-pos', (req, res) => {
  var requestSettings = {
    method: 'GET',
    url: 'https://gtfsrt.api.translink.com.au/api/realtime/SEQ/VehiclePositions',
    encoding: null
  };

  var number = req.query.numberOfVehicle;
  request(requestSettings, function (error, response, body) {
    if (!error && response.statusCode == 200) {
      var feed = GtfsRealtimeBindings.transit_realtime.FeedMessage.decode(body);
      var data = [];
      for(var i = 0; i < number; i++)
        data.push(feed.entity[i]);
      res.send(data);
    }
  });
});

app.get('/gtfsrt-trip-update', (req, res) => {
  var requestSettings = {
    method: 'GET',
    url: 'https://gtfsrt.api.translink.com.au/api/realtime/SEQ/TripUpdates',
    encoding: null
  };

  request(requestSettings, function (error, response, body) {
    if (!error && response.statusCode == 200) {
      var feed = GtfsRealtimeBindings.transit_realtime.FeedMessage.decode(body);
      res.send(feed.entity);
      // feed.entity.forEach(function(entity) {
      //   if (entity.trip_update) {
      //     console.log(entity.trip_update);
      //   }
      // });
    }
  });
});

app.get('/gtfsrt-alerts', (req, res) => {
  var requestSettings = {
    method: 'GET',
    url: 'https://gtfsrt.api.translink.com.au/api/realtime/SEQ/Alerts',
    encoding: null
  };
  request(requestSettings, function (error, response, body) {
    if (!error && response.statusCode == 200) {
      var feed = GtfsRealtimeBindings.transit_realtime.FeedMessage.decode(body);
      res.send(feed.entity);
    }
  });
});

// Server listening
app.listen(port, () => {
  console.log(`Example app listening on port ${port}...`)
});