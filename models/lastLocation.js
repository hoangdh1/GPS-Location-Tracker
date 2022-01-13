const mongoose = require('mongoose')

const lastLocationSchema = new mongoose.Schema({
    name:{type: String,
        required: true,
        unique:true
    },
  lastLocation:{
    type: String,
    required: true
  }
},{timestamps:true})

module.exports = mongoose.model('lastLocation', lastLocationSchema)