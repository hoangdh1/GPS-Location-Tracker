require("dotenv").config();

const express = require("express");
const app = express();
const cors = require("cors");
const server = require("http").createServer(app);
app.use(cors());

//object for 1 item (update with array when using many items)
let newGeoJson;
//
// app.use(express.static(__dirname + "clients"));

const mongoose = require("mongoose");
mongoose.connect(process.env.DATABASE_URL, {
  useNewUrlParser: true,
  useUnifiedTopology: true,
});
mongoose.set("useCreateIndex", true);
mongoose.set("useFindAndModify", false);
const db = mongoose.connection;
db.on("error", (error) => console.error(error));
db.once("open", () => console.log("Connected to Mongoose"));

const lastLocation = require("./models/lastLocation");

app.get("/lastlocation", async (req, res) => {
  if (db.readyState == 1) {
    try {
      const existData = await lastLocation.find({}).sort({ _id: -1 });
      res.status(200).send(existData);
    } catch (e) {
      res.send(e);
    }
  }
});

const io = require("socket.io")(server, {
  cors: {
    origin: "*",
    methods: "GET,HEAD,PUT,PATCH,POST,DELETE",
    preflightContinue: false,
    optionsSuccessStatus: 204,
  },
});

// Simulation from UI -> server -> esp on/off mode
io.on("onSimulateFromUI", (socket) => {
  socket.emit("onSimulateFromServer", "on"); // off simulate from server to esp
  console.log("onSimulateFromUI");
});
io.on("offSimulateFromUI", (socket) => {
  socket.emit("offSimulateFromServer", "off"); // off simulate from server to esp
  console.log("offSimulateFromUI");
});

io.on("connection", (socket) => {
  socket.emit("hi", "Xin chuc mung ket noi thanh cong");

  socket.on("sendData", (data) => {
    newGeoJson = JSON.stringify(data.data);

    console.log(typeof newGeoJson);
    console.log(newGeoJson);

    io.emit("renderData", { data: data.data, date: data.date });
  });
  socket.on("disconnect", async () => {
    if (newGeoJson !== undefined) {
      try {
        let data = JSON.parse(newGeoJson);
        const lastData = new lastLocation({
          name: data.timestamp,
          title: data.properties.title,
          lastLocation: newGeoJson,
        });

        // Updata last location
        // const savedData = await lastLocation.findOneAndUpdate(
        //   { name: lastData.name },
        //   { lastLocation: lastData.lastLocation }
        // );
        // if (savedData === null) {
        //   await lastData.save();
        // }

        // Save new last location
        await lastData.save();

        console.log("Disconnected and save last location");
      } catch (error) {
        console.log(error);
      }
    }
  });
});

// console.log(Date.now());
server.listen(process.env.PORT || 4000, () =>
  console.log(`Server has started in port ${process.env.PORT}`)
);
