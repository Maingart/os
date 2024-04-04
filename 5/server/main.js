const express = require('express');
const cors = require('cors');
const routes = require('./routes');
const errorHandler = require('./errorHandler');
const logger = require('./logger');

const app = express();
const port = 1337;

app.use(cors());
app.use(logger); // подключаем middleware для логирования запросов
app.use('/', routes);
app.use(errorHandler); // подключаем middleware для обработки ошибок

app.listen(port, () => {
    console.log(`http://localhost:${port}`);
});
