const express = require("express");
const http = require("http");

const app = express();
const server = http.createServer(app);

const io = require("socket.io")(server);

app.use(express.json());
app.use(express.static("public"));

let sensorData = {
  temperature: 0,
  humidity: 0,
  thermistor: 0,
  pir: false,
  sound: false,
};

app.post("/data", (req, res) => {
  sensorData = req.body;

  console.log("Received:");
  console.log(sensorData);

  io.emit("sensor", sensorData);

  res.sendStatus(200);
});

app.get("/sensor", (req, res) => {
  res.json(sensorData);
});

server.listen(3000, () => {
  console.log("Server running on port 3000");
});
