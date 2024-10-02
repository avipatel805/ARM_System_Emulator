



document.addEventListener("DOMContentLoaded", function() {

    var stages = ['fetch', 'decode', 'execute', 'memory', 'writeback'];
    var pipelineInfoContainer = document.getElementById('pipeline-info-container');
    var pipelineContainer = document.getElementById('pipeline-container');
    var pastData = []; // Array to store historical data
    var currentIndex = -1; // Current index in the pastData array
    
    function createCell(value) {
      var cell = document.createElement('div');
      cell.classList.add('cell');
      cell.style.display = 'inline-block'; // Ensure the cell is inline      
      cell.textContent = value;
      return cell;
    }

    function createHexPrefix(value) {
      var cell = document.createElement('div');
      cell.classList.add('hex_prefix');
      cell.style.display = 'inline-block'; // Ensure the cell is inline      
      cell.textContent = value;
      return cell;
    }    
  
    function createCellRow(hexString) {
      var container = document.createElement('div');
      container.classList.add('cell-container');
      // Assuming the string is already properly formatted as 8 chunks
      hexString.match(/.{1,2}/g).forEach(function(chunk) {
        container.appendChild(createCell(chunk));
      });
      // add a breakline in container
      return container;
    }
  
    function createHexNumber(value) {
      if (value.startsWith('0x')) {
        var hexNumSpan = document.createElement('span');
        hexNumSpan.classList.add('hex-number');
        hexNumSpan.textContent = value;
        return hexNumSpan.outerHTML;
      }
      return value; // Return value as is if it's not a hex number
    }
    
    function createOpElement(value, className) {
      if (className == "alu-op") {
        var opSpan = document.createElement('span');
        opSpan.classList.add(className);
        opSpan.textContent = "alu-op: " + value.trim(); // Trim for aesthetic uniformity
        return opSpan.outerHTML;
      }

      var opSpan = document.createElement('span');
      opSpan.classList.add(className);
      opSpan.textContent = value.trim(); // Trim for aesthetic uniformity
      return opSpan.outerHTML;
    }
  
    function createHexCellRow(hexString) {
      var container = document.createElement('div');
      container.classList.add('cell-container');
      // container.style.display = 'inline-block'
      // container.style.flexWrap = 'wrap'; // Allow items to wrap
      container.style.width = 'max-content'; // Set a fixed width based on content
      
      // Remove the '0x' prefix if present and make sure the string has even length
      var cleanHexString = hexString.replace('0x', '');
      // Add leading zero if the string length is odd
      if (cleanHexString.length % 2 !== 0) {
        cleanHexString = '0' + cleanHexString;
      }
    
      // Split the clean hex string into pairs of characters
      var matches = cleanHexString.match(/.{1,2}/g) || [];
      
      // Fill the rest of the cells with zeros if there are less than 8 cells
      // while(matches.length < 8) {
      //   matches.unshift('00');
      // }
    
      // Create a cell for each pair
      container.appendChild(createHexPrefix("0x"));
      matches.forEach(function(pair) {
        container.appendChild(createCell(pair.toUpperCase()));
      });
    
      return container;
    }

    function visualizeStringWithColor(inputString) {
      // Define the segments and their corresponding CSS classes
      const segments = {
          'AOK': 'text-lightgreen',
          'HLT': 'text-lightgreen',
          'INS': 'text-lightcoral',
          'ADR': 'text-lightcoral',
          'BUB': 'text-lightblue',
      };
  
      // Create a container for the visualized string
      const container = document.createElement('div');
      container.classList.add('cell-container');
      container.style.display = 'inline-block'
      // Split the input string into segments that match and don't match the keys
      const regex = new RegExp(`(${Object.keys(segments).join('|')})`, 'g');
      const parts = inputString.split(regex);
  
      parts.forEach(part => {
          // Create a span element for each part
          const span = document.createElement('span');
          span.textContent = part;
  
          // If the part matches one of the segments, apply the corresponding CSS class
          if (segments[part]) {
              span.className = segments[part];
          }
  
          // Append the span to the container
          container.appendChild(span);
      });
  
      // Append the container to the body or to a specific element in your page
      return container;
  }

  function getPIPECellContent(content) {
    console.log(content.status, content.opcode)
    if (content.status == "AOK" || content.status == "HLT") {
      return content.opcode;      
    }
    else {
      return content.status;
    }
  }

  function addPIPERow(cycle, instrIn, f, d, x, m, w, instrOut) {
    // Access the table element
    var table = document.getElementById('dynamicTable');
    console.log("Add PIPERow")
    // Insert a row at the end of the table
    var newRow = table.insertRow(-1);
    // Insert cells into the row
    var cellCycle = newRow.insertCell(0);
    // var cellInstrIn = newRow.insertCell(1);
    var cellF = newRow.insertCell(1);
    var cellD = newRow.insertCell(2);
    var cellX = newRow.insertCell(3);
    var cellM = newRow.insertCell(4);
    var cellW = newRow.insertCell(5);
    // var cellInstrOut = newRow.insertCell(7);
    
    // Add text to the cells
    cellCycle.innerHTML = cycle;
    // cellInstrIn.innerHTML = getPIPECellContent(instrIn.content);
    cellF.innerHTML = getPIPECellContent(f.content);
    cellD.innerHTML = getPIPECellContent(d.content);
    cellX.innerHTML = getPIPECellContent(x.content);
    cellM.innerHTML = getPIPECellContent(m.content);
    cellW.innerHTML = getPIPECellContent(w.content);
    // cellInstrOut.innerHTML = getPIPECellContent(instrOut.content);
    
    // Apply styles to cells if specified
    // if (instrIn.class) cellInstrIn.className = instrIn.class;
    if (f.class) cellF.className = f.class;
    if (d.class) cellD.className = d.class;
    if (x.class) cellX.className = x.class;
    if (m.class) cellM.className = m.class;
    if (w.class) cellW.className = w.class;

    // hide all the rows that are larger than cycle
  }

  function updateDynamicTable(cycle) {
    table = document.getElementById('dynamicTable');
    var rows = table.rows;
    console.log("Number of rows", rows.length, cycle)
    for (var i = 1; i < rows.length; i++) {
      if (i > cycle + 1) {
        rows[i].style.display = "none";
      } else {
        rows[i].style.display = "";
      }
      console.log("Row: ", i, rows[i].style.display)

    }
  }

    function insertStageInfo(stageData) {
      return Object.keys(stageData).map(function(key) {
        if (key == "opcode") {
          // This will check for 'opcode' or 'alu_op'
          return "";
        } else if (key == "alu_op") {
          return createOpElement(stageData[key], 'alu-op');
        } else if (key.endsWith('_imm') || key.endsWith('_ex') || key.endsWith('_mem') || key.endsWith('_val_a') || key.endsWith('_val_b') || key.endsWith('_hw') || key == "pcbits" || key == "insnbits" || key == "seqsuccpc" || key =="predpc") {
          // This will check for hex number values which should be split into memory cells
          return key + ': <br> ' + createHexCellRow(stageData[key]).outerHTML;
        }
        else if (key == "status" || key == "stage") {
          return ""
        }
        return key + ': ' + stageData[key]; // For other values, return as is
      }).join('<br>'); // Use line breaks for separation in the HTML
    }

    function adjustGridColumnsToTable() {
      const instructionHeaders = document.querySelectorAll('.instruction-header');
      let gridTemplateColumnsValue = '';
  
      instructionHeaders.forEach((header, index) => {
          if (index > 0 && index <= 5) { // Assuming the first 5 instruction headers correspond to F, D, X, M, W
              const width = header.offsetWidth;
              gridTemplateColumnsValue += `${width}px `;
          }
      });
  
      const pipelineContainer = document.querySelector('.pipeline-container');
      pipelineContainer.style.gridTemplateColumns = gridTemplateColumnsValue.trim();
  }
    
    var ws = new WebSocket('ws://localhost:8080');
    var dataContainer = document.getElementById('data-container');
    
    ws.onopen = function() {
        console.log('WebSocket connected');
    };
    
    function displayData(index) {
        var data = pastData[index];
        console.log(index)
        // ... code to populate pipelineContainer with data ...
        // pipelineContainer.innerHTML = `${index < pastData.length - 1 ? `<div class="cycle-text">Previous Cycle: ${index}</div><br>` : `<div class="cycle-text">Current Cycle: ${index}</div><br>`}`

        pipelineInfoContainer.innerHTML = '';
        // visualize cycle number 
        var cycleDiv = document.createElement('div');
        cycleDiv.classList.add('cycle-text');
        // highlight the current cycle with a different color
        cycleDiv.innerHTML = `${index < pastData.length - 1 ? `<span class="previous-cycle">Previous Cycle: ${index}</span>` : `<span class="current-cycle">Current Cycle: ${index}</span>`}`;
        pipelineInfoContainer.appendChild(cycleDiv);

        pipelineContainer.innerHTML = '';

        // add two placeholder stage
        // var stageDiv = document.createElement('div');
        // stageDiv.classList.add('stage-placeholder');
        // stageDiv.style.width = document.querySelector('.cycle-header').offsetWidth + 'px';
        // pipelineContainer.appendChild(stageDiv);
        // stageDiv = document.createElement('div');
        // stageDiv.classList.add('stage-placeholder');
        // stageDiv.style.width = document.querySelector('.instruction-header').offsetWidth + 'px';
        // pipelineContainer.appendChild(stageDiv);
        for (let i = 0; i < 1; i++) {
          let placeholderDiv = document.createElement('div');
          // Optionally, add a class to control visibility or styling
          placeholderDiv.classList.add('stage-placeholder');
          pipelineContainer.appendChild(placeholderDiv);
        }
        
        stages.forEach(function(stage) {
            var stageDiv = document.createElement('div');
            stageDiv.classList.add('stage');
            var additionalInfo = '';
            // set width of the stage div to same as instruction-header
            // stageDiv.style.width = document.querySelector('.instruction-header').offsetWidth + 'px';
            console.log(stage, stageDiv.style.width, stageDiv.style.left);
            Object.keys(data[stage]).forEach(function(key) {
              if (key !== 'stage' && key !== 'opcode' && key !== 'pcbits' && key !== 'insnbits') {
                additionalInfo += key + ': ' + data[stage][key] + '\n';
              }
            });

            stageDiv.innerHTML = `
              <div class="stage-title">${stage.toUpperCase()}</div>
              <div class="opcode"> ${data[stage].opcode}</div>
              <div class="stage-info"> status:${visualizeStringWithColor(data[stage].status).outerHTML}</div>
              <div class="stage-info">${insertStageInfo(data[stage])}</div>
              `;
            pipelineContainer.appendChild(stageDiv);
          });         
         
          // ${data[stage].pcbits ? "pcbits:" + createCellRow(data[stage].pcbits).outerHTML: ''}
          // <div></div>
          // ${data[stage].insnbits ? "insbits: " + createCellRow(data[stage].insnbits).outerHTML : ''}

          // Create a new text box for each message
          var textBox = document.createElement('textarea');
          textBox.classList.add('data-box');
          textBox.rows = 10;
          textBox.cols = 50;
          // Format the JSON data with indentation for readability
          textBox.value = JSON.stringify(data, null, 2);
          textBox.readOnly = true; // Make the text box read-only
          dataContainer.appendChild(textBox);
    }

    ws.onmessage = function(event) {
        try {
            console.log('Received raw data:', event.data);
            var data = JSON.parse(event.data);
            console.log('Received data:', data);
            pastData.push(data); // Store the received data
            currentIndex = pastData.length - 1; // Update the current index to the latest data
            displayData(currentIndex); // Function to display data for a given index
            const segments = {
              'AOK': 'white-cell',
              'HLT': 'green-cell',
              'INS': 'red-cell',
              'ADR': 'red-cell',
              'BUB': 'purple-cell',
          };
          addPIPERow(
            currentIndex,
            { content: pastData[currentIndex].fetch, class: segments["AOK"]},
            { content: pastData[currentIndex].fetch, class: segments[pastData[currentIndex].fetch.status] },
            { content: pastData[currentIndex].decode, class: segments[pastData[currentIndex].decode.status] },
            { content: pastData[currentIndex].execute, class: segments[pastData[currentIndex].execute.status] },
            { content: pastData[currentIndex].memory, class: segments[pastData[currentIndex].memory.status] },
            { content: pastData[currentIndex].writeback, class: segments[pastData[currentIndex].writeback.status] },
            
            { content: pastData[currentIndex].writeback, class: 'purple-icon' },
            'I1'
          );
        } catch (e) {
            console.log('Error parsing JSON:', e);
        }
    };
    
    ws.onclose = function() {
        console.log('WebSocket disconnected');
    };

    ws.onerror = function(err) {
        console.log('WebSocket error:', err);
    };

    ws.onload = function() {
      adjustGridColumnsToTable();
    }

    ws.onresize = function() {
      adjustGridColumnsToTable();
    }

    document.addEventListener('keydown', function(event) {
        const keyInfo = { key: event.key };
        if (currentIndex < pastData.length - 1 & (event.key === 'ArrowRight' || event.key === 'ArrowDown')) {
          currentIndex += 1;
            // Navigate to the next data entry
            displayData(currentIndex);
            updateDynamicTable(currentIndex);
            // exit the function

            return;
        }
        ws.send(JSON.stringify(keyInfo));
        console.log('Sent key:', event.key);
        if (event.key === 'ArrowLeft' || event.key === 'ArrowUp') {
            // Navigate to the previous data entry
            if (currentIndex > 0) {
                currentIndex -= 1;
                displayData(currentIndex);
                updateDynamicTable(currentIndex);
            }
        } else if (event.key === 'Enter') {
            // Navigate to the latest data entry
            currentIndex = pastData.length - 1;
            displayData(currentIndex);
            updateDynamicTable(currentIndex);
        }
    });

    document.getElementById('screenshotBtn').addEventListener('click', function() {
        // Use document.body for a full-page screenshot or a specific element for a section
        html2canvas(document.body).then(function(canvas) {
            // Create an image from the canvas and download it
            var link = document.createElement('a');
            link.download = 'screenshot.png';
            link.href = canvas.toDataURL('image/png');
            link.click();
        });
    });
    let mediaRecorder;
    let recordedChunks = [];
    
    document.getElementById('startRecordingBtn').addEventListener('click', async () => {
        // Ask the user for permission to capture the screen
        try {
            const stream = await navigator.mediaDevices.getDisplayMedia({
                video: { mediaSource: 'screen' }
            });
    
            // Set up the MediaRecorder
            mediaRecorder = new MediaRecorder(stream);
            mediaRecorder.ondataavailable = handleDataAvailable;
            mediaRecorder.onstop = handleStop;
            mediaRecorder.start();
    
            // Disable the start button and enable the stop button
            document.getElementById('startRecordingBtn').disabled = true;
            document.getElementById('stopRecordingBtn').disabled = false;
        } catch (err) {
            // Handle errors (user denied permission, etc.)
            console.error("Error: " + err);
        }
    });
    
    document.getElementById('stopRecordingBtn').addEventListener('click', () => {
        // Stop the recording
        mediaRecorder.stop();
        // Enable the start button and disable the stop button
        document.getElementById('startRecordingBtn').disabled = false;
        document.getElementById('stopRecordingBtn').disabled = true;
    });
    
    function handleDataAvailable(event) {
        if (event.data.size > 0) {
            recordedChunks.push(event.data);
        }
    }
    
    function handleStop() {
        const blob = new Blob(recordedChunks, {
            type: 'video/webm; codecs=vp9'
        });
        recordedChunks = [];
    
        const url = URL.createObjectURL(blob);
        // Create an anchor tag to download the video file
        const a = document.createElement('a');
        a.href = url;
        a.download = 'recording.webm';
        document.body.appendChild(a);
        a.click();
        // Cleanup
        window.URL.revokeObjectURL(url);
        document.body.removeChild(a);
    }
        
});


