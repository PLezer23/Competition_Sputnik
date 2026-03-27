<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Спутник - мониторинг</title>
    <style>
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        
        h1 {
            text-align: center;
            color: white;
            margin-bottom: 30px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        
        .stats {
            background: white;
            border-radius: 20px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }
        
        .station {
            background: #f8f9fa;
            border-left: 5px solid #28a745;
            border-radius: 10px;
            padding: 15px;
            margin: 15px 0;
            transition: transform 0.2s;
        }
        
        .station:hover {
            transform: translateX(5px);
        }
        
        .station h3 {
            margin-top: 0;
            color: #333;
            border-bottom: 1px solid #ddd;
            padding-bottom: 10px;
        }
        
        .connected {
            color: #28a745;
            font-weight: bold;
        }
        
        .disconnected {
            color: #dc3545;
            font-weight: bold;
        }
        
        .data-row {
            display: flex;
            justify-content: space-between;
            margin: 10px 0;
            padding: 5px 0;
            border-bottom: 1px solid #eee;
        }
        
        .label {
            color: #666;
        }
        
        .value {
            font-weight: bold;
            color: #333;
        }
        
        .gas-level {
            width: 100%;
            height: 10px;
            background: #e9ecef;
            border-radius: 5px;
            margin: 10px 0;
        }
        
        .gas-fill {
            height: 100%;
            background: linear-gradient(90deg, #28a745, #ffc107, #dc3545);
            border-radius: 5px;
            width: 0%;
        }
        
        .timestamp {
            text-align: center;
            color: white;
            margin-top: 20px;
            font-size: 0.9em;
        }
        
        button {
            background: white;
            color: #667eea;
            border: none;
            padding: 15px 30px;
            font-size: 18px;
            border-radius: 50px;
            cursor: pointer;
            width: 100%;
            font-weight: bold;
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
            transition: transform 0.2s;
        }
        
        button:hover {
            transform: scale(1.05);
        }
        
        .last-update {
            text-align: center;
            color: #ddd;
            margin-top: 10px;
            font-size: 0.8em;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🛰️ Мониторинг спутника</h1>
        
        <div class="stats">
            <div style="text-align: center; margin-bottom: 20px;">
                <span style="font-size: 24px;">📡 Статус: </span>
                <span id="connectionStatus" style="font-size: 24px; font-weight: bold; color: #28a745;">Подключено</span>
            </div>
            
            <div id="stationsContainer">
                <!-- Сюда будут добавляться станции -->
                <p style="text-align: center; color: #999;">Загрузка данных...</p>
            </div>
        </div>
        
        <button onclick="releaseStation()">🚀 Сбросить станцию</button>
        
        <div class="last-update" id="lastUpdate">
            Последнее обновление: --
        </div>
    </div>

    <script>
        // ============ ПЕРЕМЕННЫЕ ============
        let updateTimer;
        let lastUpdateTime = '--';
        
        // ============ ЗАПУСК ПРИ ЗАГРУЗКЕ ============
        window.onload = function() {
            getData();  // Сразу получаем данные
            updateTimer = setInterval(getData, 3000);  // И каждые 3 секунды обновляем
        };
        
        // ============ ПОЛУЧЕНИЕ ДАННЫХ СО СПУТНИКА ============
        function getData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    updateDisplay(data);
                    
                    // Обновляем время
                    let now = new Date();
                    lastUpdateTime = now.getHours() + ':' + 
                                   String(now.getMinutes()).padStart(2, '0') + ':' + 
                                   String(now.getSeconds()).padStart(2, '0');
                    document.getElementById('lastUpdate').innerHTML = 'Последнее обновление: ' + lastUpdateTime;
                })
                .catch(error => {
                    console.error('Ошибка:', error);
                    document.getElementById('connectionStatus').innerHTML = 'Ошибка связи';
                    document.getElementById('connectionStatus').style.color = '#dc3545';
                });
        }
        
        // ============ ОТОБРАЖЕНИЕ ДАННЫХ ============
        function updateDisplay(data) {
            let container = document.getElementById('stationsContainer');
            container.innerHTML = '';  // Очищаем
            
            if(data.stationCount === 0) {
                container.innerHTML = '<p style="text-align: center; color: #999;">Нет подключенных станций</p>';
                return;
            }
            
            // Добавляем каждую станцию
            for(let i = 0; i < data.stations.length; i++) {
                let station = data.stations[i];
                
                let stationHtml = '<div class="station">';
                stationHtml += '<h3>' + station.name + '</h3>';
                
                if(station.connected) {
                    stationHtml += '<p class="connected">✅ Подключена</p>';
                    
                    // Температура
                    stationHtml += '<div class="data-row">';
                    stationHtml += '<span class="label">🌡️ Температура:</span>';
                    stationHtml += '<span class="value">' + station.temp.toFixed(1) + ' °C</span>';
                    stationHtml += '</div>';
                    
                    // Влажность
                    stationHtml += '<div class="data-row">';
                    stationHtml += '<span class="label">💧 Влажность:</span>';
                    stationHtml += '<span class="value">' + station.hum.toFixed(1) + ' %</span>';
                    stationHtml += '</div>';
                    
                    // Газ
                    stationHtml += '<div class="data-row">';
                    stationHtml += '<span class="label">🧪 Качество воздуха:</span>';
                    stationHtml += '<span class="value">' + station.gas + '%</span>';
                    stationHtml += '</div>';
                    
                    // Полоска газа
                    let gasColor = 'green';
                    if(station.gas > 70) gasColor = 'red';
                    else if(station.gas > 40) gasColor = 'orange';
                    
                    stationHtml += '<div class="gas-level">';
                    stationHtml += '<div class="gas-fill" style="width: ' + station.gas + '%; background: ' + gasColor + ';"></div>';
                    stationHtml += '</div>';
                    
                } else {
                    stationHtml += '<p class="disconnected">❌ Нет связи</p>';
                }
                
                stationHtml += '</div>';
                container.innerHTML += stationHtml;
            }
        }
        
        // ============ СБРОС СТАНЦИИ ============
        function releaseStation() {
            fetch('/release')
                .then(response => response.text())
                .then(data => {
                    alert(data);
                })
                .catch(error => {
                    alert('Ошибка: ' + error);
                });
        }
    </script>
</body>
</html>
