
import express from 'express';
import rateLimit from 'express-rate-limit';
import errorHandler from 'errorhandler';
import morgan from 'morgan';
import bodyParser from 'body-parser';
import sqlite3 from 'sqlite3';

import { readFileSync } from 'fs';

const app = express();
const port = 39839;

const COLE_LOCAL = false;
const FS_DB = COLE_LOCAL ? "./db.db" : "/secrets/db.db";
const FS_INIT_SQL = COLE_LOCAL ? "C:/Users/ColeNelson/Desktop/cs839-hack/cs839-hack-api/includes/init.sql" : "/secrets/init.sql";

const INIT_SQL = readFileSync(FS_INIT_SQL).toString();

const GET_CRIMINALS_SQL = `SELECT * FROM BadgerCriminal;`
const POST_CRIMINALS_SQL = `INSERT INTO BadgerCriminal(drunken, person, metatext, img, datetaken) VALUES(?, ?, ?, ?, ?);`;

const db = await new sqlite3.Database(FS_DB, sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE, (err) => {
    if (err) {
        console.log("Failed to create/open SQL database!");
        exit(1);
    } else {
        console.log("Created/opened SQL database!")
    }
});
db.serialize(() => {
    INIT_SQL.replaceAll(/\t\r\n/g, ' ').split(';').filter(str => str).forEach((stmt) => db.run(stmt + ';'));
});

// LOGGING
app.use(morgan((tokens, req, res) => {
    return [
        tokens.date(),
        tokens['remote-addr'](req, res),
        tokens.method(req, res),
        tokens.url(req, res),
        tokens.status(req, res),
        tokens['response-time'](req, res), 'ms'
    ].join(' ')
}));

app.use(express.json({limit: '2mb'}));


morgan.token('date', function () {
    var p = new Date().toString().replace(/[A-Z]{3}\+/, '+').split(/ /);
    return (p[2] + '/' + p[1] + '/' + p[3] + ':' + p[4] + ' ' + p[5]);
});

process.on('uncaughtException', function (exception) {
    console.log(exception);
});

process.on('unhandledRejection', (reason, p) => {
    console.log("Unhandled Rejection at: Promise ", p, " reason: ", reason);
});

app.use(errorHandler({ dumpExceptions: true, showStack: true }));

// JSON Body Parser Configuration
app.use(bodyParser.urlencoded({
    extended: true
}));
app.use(bodyParser.json());

// Request Throttler
app.set('trust proxy', 1);

// Allow CORS
app.use((req, res, next) => {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "X-CS571-ID, Origin, X-Requested-With, Content-Type, Accept");
    next();
});

// Throttling
app.use(rateLimit({
    message: {
        msg: "Too many requests, please try again later."
    },
    windowMs: 30 * 1000, // 30 seconds
    max: (req, res) => req.method === "OPTIONS" ? 0 : 100, // limit each client to 100 requests every 30 seconds
    keyGenerator: (req, res) => req.header('X-CS571-ID') // throttle on BID
}));

// Endpoints Go Here!
app.get('/api/criminals', (req, res) => {
    db.prepare(GET_CRIMINALS_SQL).run().all((err, rows) => {
        if (!err) {
            res.status(200).send({
                msg: "Successfully got the latest criminals!",
                criminals: rows
            });
        } else {
            res.status(500).send({
                msg: "The operation failed. The error is provided below. This may be server malfunction; check that your request is valid, otherwise contact CS571 staff.",
                error: err
            });
        }
    });
});

app.post('/api/criminals', (req, res) => {
    db.prepare(POST_CRIMINALS_SQL).get(req.body.drunken, "", "", req.body.img, new Date(), (err, resp) => {
        if (!err) {
            res.status(200).send({
                msg: "Successfully added criminal to database!"
            })
        } else {
            res.status(500).send();
        }
    });
});


// Error Handling
app.use((err, req, res, next) => {
    let datetime = new Date();
    let datetimeStr = `${datetime.toLocaleDateString()} ${datetime.toLocaleTimeString()}`;
    console.log(`${datetimeStr}: Encountered an error processing ${JSON.stringify(req.body)}`);
    res.status(500).send({
        "error-msg": "Oops! Something went wrong. Check to make sure that you are sending a valid request. Your recieved request is provided below. If it is empty, then it was most likely not provided or malformed. If you have verified that your request is valid, please contact the CS571 staff.",
        "error-req": JSON.stringify(req.body),
        "date-time": datetimeStr
    })
});

// Open Server for Business
app.listen(port, () => {
    console.log(`CS571 API :${port}`)
});
