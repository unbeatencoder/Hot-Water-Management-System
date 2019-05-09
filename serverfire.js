var myapp = require("firebase");
const express = require('express');
const bodyParser = require('body-parser');
const app=express();
const cors=require('cors');
const schedule = require('node-schedule');

app.use(cors());
app.use(bodyParser.json());;
var config = {
    apiKey: "AIzaSyCOiV6cPER2Gl9dCCm8VP0Zga0jYRlOQus",
    authDomain: "iot-data-aeed1.firebaseapp.com",
    databaseURL: "https://iot-data-aeed1.firebaseio.com",
    projectId: "iot-data-aeed1",
    storageBucket: "iot-data-aeed1.appspot.com",
    messagingSenderId: "361006057981"
  };

myapp.initializeApp(config);

var auth=myapp.auth();
var database=myapp.database();

app.post('/signin',(req,res)=>{
    if(req.body.email==="admin@email.com")
    {
      auth.signInWithEmailAndPassword(req.body.email,req.body.password)
      .then((cuser)=>{
              res.json(cuser.user.uid);
      })
      .catch((error)=>res.status(400).json());
    }
    else
      res.status(400).json();
});
app.post('/timedata',(req,res)=>{
  database.ref("ARCHIVES").once('value').then((snapshot)=>{
    let kk=req.body.floor;
    if(kk!=null && snapshot.val().hasOwnProperty(kk))
      res.json(snapshot.val()[kk].timestamped_measures);
    else
      res.json("yo");
  })
})
app.post('/data',(req,res)=>{
    database.ref("DATA").once('value').then((snapshot)=>{
      res.json(snapshot.val());
    })
});
schedule.scheduleJob('49 16 * * *', ()=>{
  database.ref("FLOORMEMBERS").once("value").then((snapshot)=>{
    Object.entries(snapshot.val()).forEach((floor)=>{
      let kk=floor[0];let mvote=0;let mvotei=-1;let ff=floor[1];
      for(let i=0;i<=22;i+=2)
      {
        let vote=0;
        Object.values(ff).forEach(pp=>{
          mm=pp.preference.currentpreference;
          let ll=Number(mm.split("-")[0].length);let cc;
          if(mm.split("-")[0].includes(":"))
            {cc=Number(mm.split("-")[0].substring(0,ll-5));cc+=0.5;}
          else
            cc=Number(mm.split("-")[0].substring(0,ll-2));
          if(mm.split("-")[0].charAt(ll-2)=='p' || mm.split("-")[0].charAt(ll-2)=='P') cc+=12;
          if(cc>=i && cc<i+2)  vote++;
        })
        if(vote>mvote)
        {
          mvote=vote;mvotei=i;
        }
      }
      var obj = {};
      obj[kk] = mvotei;
      database.ref("schedule").update(obj);
    })
  })
  
});
schedule.scheduleJob('27 16 * * *', ()=>{
  database.ref("SLOTUSED").once("value").then((snapshot)=>{
    Object.entries(snapshot.val()).forEach((user)=>{
      let obj=new Object();
      obj[user[0]]="1";
      database.ref("SLOTUSED").update(obj);
    })
  })
})
schedule.scheduleJob('40 0 * * *', ()=>{
  database.ref("ARCHIVES").set({});
})
schedule.scheduleJob('12 16 * * *', ()=>{
    let today = new Date();
    let tomorrow = new Date();
    tomorrow.setDate(today.getDate()+1);
    let weekdays = new Array(
      "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"
    );
    let kkk=weekdays[tomorrow.getDay()];let ans=-1;
    database.ref("FLOORMEMBERS").once("value").then((snapshot)=>{
      Object.entries(snapshot.val()).forEach((floor)=>{
        Object.entries(floor[1]).forEach(pp=>{
          ans=pp[1].preferences[kkk];
          database.ref("FLOORMEMBERS").child(floor[0]).child(pp[0]).child("preference").update({currentpreference:ans});
        })
      })
    })
});

app.listen(3001,()=>{
    console.log("app is running in port 3001");
});