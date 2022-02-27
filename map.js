var existData = [];
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function () {
  if (this.readyState == 4 && this.status == 200) {
    let data = JSON.parse(this.response);
    existData.push(JSON.parse(data[0].lastLocation));
  }
};
xhttp.open("GET", "http://localhost:4000/lastlocation", true);
xhttp.send();

mapboxgl.accessToken =
  "pk.eyJ1IjoiaG9hbmdkaCIsImEiOiJjbDAzYTBvZHgxMmZxM2RucDIyZ3Jyam1kIn0.O9TETWvxngtVYrT_mr2LBQ";

var map = new mapboxgl.Map({
  container: "map", // container ID
  style: "mapbox://styles/mapbox/streets-v11", // style URL
  center: [105.835856, 21.012236], // starting position [lng, lat]
  zoom: 7, // starting zoom
});

const socket = io("http://localhost:4000");
map.on("load", function () {
  map.flyTo({
    center: existData[0].geometry.coordinates,
    speed: 5,
    zoom: 16,
  });

  if (map.hasImage("custom-marker")) map.removeImage("custom-marker");
  if (map.getLayer("points")) map.removeLayer("points");
  map.loadImage("bus.png", function (error, image) {
    if (error) throw error;
    map.addImage("custom-marker", image);
    map.addSource("points", {
      type: "geojson",
      data: {
        type: "FeatureCollection",
        features: existData,
      },
    });
    map.addLayer({
      id: "points",
      type: "symbol",
      source: "points",
      layout: {
        "icon-image": "custom-marker",
        "text-field": ["get", "title"],
        "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
        "text-offset": [0, 1.25],
        "text-anchor": "top",
      },
    });
  });
  socket.on("renderData", (data) => {
    console.log(typeof data.data);

    let json = data.data;

    map.flyTo({
      center: json.geometry.coordinates,
      speed: 5,
      zoom: 16,
    });
    if (map.hasImage("custom-marker")) map.removeImage("custom-marker");
    if (map.getLayer("points")) map.removeLayer("points");
    map.loadImage("bus.png", function (error, image) {
      if (error) throw error;
      map.addImage("custom-marker", image);

      map.getSource("points").setData(json);

      map.addLayer({
        id: "points",
        type: "symbol",
        source: "points",
        layout: {
          "icon-image": "custom-marker",
          "text-field": ["get", "title"],
          "text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
          "text-offset": [0, 1.25],
          "text-anchor": "top",
        },
      });
    });
  });
  // Add an image to use as a custom marker
});

// Simulation
const checkbox = document.querySelector("#checkbox");
const body = document.body;

checkbox.addEventListener("change", (e) => {
  if (e.target.checked) {
    body.style.background = "#FFFF00";
  } else {
    document.body.style.background = "#000000";
  }
});
