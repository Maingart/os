const express = require('express');
const { executeQuery } = require('./db');

const router = express.Router();

const errorHandler = (res, error) => {
    console.error('Error executing query:', error);
    res.status(500).json({ error: 'Internal Server Error' });
};

router.get('/getCurrentTemperature', async (req, res) => {
    try {
        const tempQ = await executeQuery('SELECT temperature FROM total ORDER BY timestamp DESC LIMIT 1;');
        const result = tempQ[0].temperature;

        const responseData = {
            result,
        };

        res.json(responseData);
    } catch (error) {
        errorHandler(res, error);
    }
});

router.get('/getTemperature', async (req, res) => {
    try {
        const temperatureDataQuery = `
            SELECT id, timestamp, temperature
            FROM total`;

        const temperatureDataResult = await executeQuery(temperatureDataQuery);

        const temperatureData = temperatureDataResult.map(entry => ({
            id: entry.id,
            timestamp: entry.timestamp,
            temperature: entry.temperature,
        }));

        const responseData = {
            temperatureData,
        };

        res.json(responseData);
    } catch (error) {
        errorHandler(res, error);
    }
});

module.exports = router;
