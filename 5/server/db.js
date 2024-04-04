const sqlite3 = require('sqlite3').verbose();

const DB_NAME = process.env.DB_NAME || '../DATABASE.db';

const db = new sqlite3.Database(DB_NAME, (err) => {
    if (err) {
        console.error('Can\'t connect to database', err);
    } else {
        console.log('Connected to the database');
    }
});

const executeQuery = (query, params = []) => {
    return new Promise((resolve, reject) => {
        db.all(query, params, (err, rows) => {
            if (err) {
                reject(err);
            } else {
                resolve(rows);
            }
        });
    });
};

module.exports = { executeQuery };
