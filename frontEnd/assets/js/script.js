let ip = "192.168.113.129:5000";
let a =  document.querySelectorAll('path');
let green = '#00ff00';
let red = '#ff0000';
let yellow = '#ffff00';

function changeColor(color) {
    a[0].style.stroke = color;
    a[1].style.fill = color;
}

let count = 0;
let list = document.querySelector('.list');

//lista utenti
function createListItem(nome, cognome, id){
    
    let li = document.createElement('div');
    li.classList.add('listItem');
    li.classList.add(id);

    let name = document.createElement('div');
    name.classList.add('name');
    name.innerHTML = nome + " " + cognome + " ";

    let status = document.createElement('div');
    status.classList.add('status');
    
    let statusPlaceholder = document.createElement('div');
    statusPlaceholder.classList.add('statusPlaceholder');
    statusPlaceholder.innerHTML = 'Status: offline';

    let statusIcon = document.createElement('div');
    statusIcon.classList.add('statusIcon');
    let iconColor = "grey";
    statusIcon.innerHTML = `<svg  height="100%" viewBox="0 0 22 20" fill="none" xmlns="http://www.w3.org/2000/svg">
    <path d="M17 9.9999H16.1986C15.3689 9.9999 14.9541 9.9999 14.6102 10.1946C14.2664 10.3893 14.0529 10.745 13.6261 11.4564L13.5952 11.5079C13.1976 12.1706 12.9987 12.502 12.7095 12.4965C12.4202 12.4911 12.2339 12.1525 11.8615 11.4753L10.1742 8.4075C9.8269 7.77606 9.6533 7.46034 9.3759 7.44537C9.0986 7.43039 8.892 7.72558 8.47875 8.3159L8.19573 8.7203C7.75681 9.3473 7.53734 9.6608 7.21173 9.8303C6.88612 9.9999 6.50342 9.9999 5.73803 9.9999H5" stroke="#555" stroke-width="1.5" stroke-linecap="round"/>
    <path d="M11 3.5006L10.4596 4.0207C10.601 4.1676 10.7961 4.2506 11 4.2506C11.2039 4.2506 11.399 4.1676 11.5404 4.0207L11 3.5006ZM1.65666 11.3964C1.87558 11.748 2.33811 11.8556 2.68974 11.6367C3.04137 11.4178 3.14895 10.9552 2.93003 10.6036L1.65666 11.3964ZM5.52969 13.7718C5.23645 13.4793 4.76158 13.4798 4.46903 13.7731C4.17649 14.0663 4.17706 14.5412 4.47031 14.8337L5.52969 13.7718ZM1.75 7.13707C1.75 4.33419 3.00722 2.59507 4.57921 1.99711C6.15546 1.39753 8.35129 1.8302 10.4596 4.0207L11.5404 2.9805C9.1489 0.495831 6.3447 -0.27931 4.04591 0.59512C1.74286 1.47116 0.25 3.88785 0.25 7.13707H1.75ZM14.5026 17.4999C15.9949 16.3234 17.7837 14.7461 19.2061 12.9838C20.6126 11.2412 21.75 9.2089 21.75 7.13703H20.25C20.25 8.688 19.3777 10.3829 18.0389 12.0417C16.716 13.6807 15.0239 15.1788 13.574 16.3219L14.5026 17.4999ZM21.75 7.13703C21.75 3.88784 20.2571 1.47115 17.9541 0.59511C15.6553 -0.2793 12.8511 0.495831 10.4596 2.9805L11.5404 4.0207C13.6487 1.8302 15.8445 1.39753 17.4208 1.99711C18.9928 2.59506 20.25 4.33418 20.25 7.13703H21.75ZM7.49742 17.4998C8.77172 18.5044 9.6501 19.2359 11 19.2359V17.7359C10.2693 17.7359 9.8157 17.4174 8.42605 16.3219L7.49742 17.4998ZM13.574 16.3219C12.1843 17.4174 11.7307 17.7359 11 17.7359V19.2359C12.3499 19.2359 13.2283 18.5044 14.5026 17.4999L13.574 16.3219ZM2.93003 10.6036C2.18403 9.4054 1.75 8.2312 1.75 7.13707H0.25C0.25 8.617 0.83054 10.0695 1.65666 11.3964L2.93003 10.6036ZM8.42605 16.3219C7.50908 15.599 6.49093 14.7307 5.52969 13.7718L4.47031 14.8337C5.48347 15.8445 6.54819 16.7515 7.49742 17.4998L8.42605 16.3219Z" fill="#555"/>
    </svg>`;

    status.appendChild(statusPlaceholder);
    status.appendChild(statusIcon);

    li.appendChild(name);
    li.appendChild(status);

    list.appendChild(li);

    li.addEventListener('click', function(){
        let cards = document.querySelectorAll('.right > .card');
        let userSelected = users[id-1];
        cards.forEach(function(card){
            card.style.display = "flex";
            if (card.classList[1] == undefined ) card.classList.add(id);
            else card.classList.replace(card.classList[1], id);
            let infos = card.querySelectorAll('.info');
            
            infos[0].innerHTML = userSelected.nome + " " + userSelected.cognome + "   " + userSelected.eta + " anni";
            infos[1].innerHTML = userSelected.task;
            infos[2].innerHTML = userSelected.info;
        
        });
        console.log(userSelected.mac)
        if (userSelected.mac == ''){
            document.querySelectorAll('.subInfo > .info').forEach(function(info){
                info.style.display = "none";
            });
            document.querySelector('.missionStatus').style.display = "flex";
            document.querySelector('input[name = "macAddress"]').style.display = "flex";
            document.querySelector('.confirmMission').style.display = "flex";
        }
        else {
            document.querySelector('.missionStatus').style.display = "none";
            document.querySelector('input[name = "macAddress"]').style.display = "none";
            document.querySelector('.confirmMission').style.display = "none";
            document.querySelectorAll('.subInfo > .info').forEach(function(info){
                info.style.display = "flex";
            });
        }
        document.querySelector('.right > .placeholderRight').style.display = "none";
    });
}

document.querySelector('.confirmMission').addEventListener('click', function(){
    let mac = document.querySelector('input[name = "macAddress"]')?.value;
    let id = document.querySelector('.right > .card')?.classList[1];
    putMacAddress('http://' + ip + '/add_macAddress', {id: id, mac: mac});

    
});

function missionStarted(){
    document.querySelector('.missionStatus').style.display = "none";
    document.querySelector('input[name = "macAddress"]').style.display = "none";
    document.querySelector('.confirmMission').style.display = "none";
    document.querySelectorAll('.subInfo > .info').forEach(function(info){
        info.style.display = "flex";
    });
    
}

function missionStartError(){
    document.querySelector('input[name = "macAddress"]').style.color = "red";
    document.querySelector('input[name = "macAddress"]').classList.add('shake');
    setTimeout(function(){
        document.querySelector('input[name = "macAddress"]').classList.remove('shake');
        document.querySelector('input[name = "macAddress"]').style.color = "white";
    }, 500);
    document.querySelector('.missionStatus').innerHTML = "Mac address in utilizzo";

}


//inserimento utenti di prova
function appStart(){
    getWorkers('http://' + ip + '/get_all_names');
}

appStart();
    
//creazione dashboard
let dashboard = document.querySelector('.right');

function createDashboardItem(){
    let card = document.createElement('div');
    card.classList.add('card');

    dashboard.appendChild(card);
}


let extender = document.querySelector('.extenderUp');
let main = document.querySelector('.mainVisible');
let footer;
if (document.querySelector('.footerCompressed') != null)
 footer = document.querySelector('.footerCompressed');
else footer = document.querySelector('.footerExtended');
let map = document.querySelector('.mapCompressed');

extender.addEventListener('click', function(){
    //icon change
    extender.classList.toggle('extenderUp');
    extender.classList.toggle('extenderDown');

    //extend
    main.classList.toggle('mainVisible');
    main.classList.toggle('mainHidden');
    

    footer.classList.toggle('footerCompressed');
    footer.classList.toggle('footerExtended');
});

//add user

let addButton = document.querySelector('.addButton');
addButton.addEventListener('click', function(){
    addUser();
});

let backdropfilter = document.querySelector('.backdropFilter');
let addUserCard = document.querySelector('.addUser');
let closeButton = document.querySelector('.closeNewUser');
let confirmNewUser = document.querySelector('.confirmNewUser');

function addUser(){
    backdropfilter.style.display = "flex";
    setTimeout(function(){
        backdropfilter.style.opacity = "1";
    }, 100);

    setTimeout(function(){
        addUserCard.style.display = "flex";
    }, 500);

    setTimeout(function(){
        addUserCard.style.opacity = "1";
        addUserCard.style.scale = 1;
    }, 600);

}

closeButton.addEventListener('click', function(){
    backdropfilter.style.opacity = "0";
    setTimeout(function(){
        backdropfilter.style.display = "none";
    }, 500);

    addUserCard.style.opacity = "0";
    addUserCard.style.scale = 0;
    setTimeout(function(){
        addUserCard.style.display = "none";
    }, 500);
});

confirmNewUser.addEventListener('click', function(){
    backdropfilter.style.opacity = "0";
    setTimeout(function(){
        backdropfilter.style.display = "none";
    }, 500);

    addUserCard.style.opacity = "0";
    addUserCard.style.scale = 0;
    setTimeout(function(){
        addUserCard.style.display = "none";
    }, 500);


    let nome = document.querySelector('input[name = "nome"]').value;
    let cognome = document.querySelector('input[name = "cognome"]').value;
    let eta = document.querySelector('input[name = "eta"]').value;
    let task = document.querySelector('input[name = "task"]').value;
    let infos = document.querySelector('input[name = "info"]').value;
    putWorker('http://' + ip + '/put_worker',
    {nome: nome, cognome: cognome, eta: eta, task: task, info: infos, status: "offline"});
});

function putWorker(url, data) {
    fetch(url, {
      method: 'POST',
      body: JSON.stringify(data),
      headers: {
        'Content-Type': 'application/json'
      }
    })
    .then(response => console.log(response))
    .then(data => {
      console.log('Success:', data);
      getWorkers('http://' + ip + '/get_all_names');
    })
    .catch(error => {
        console.log('Error:', error);
    });
}

function getWorkers(url) {
        
    fetch(url, {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json'
        }
      })
      .then(response => response.json())
      .then(data => {
        console.log('Success:', data);

        if (list.childNodes.length > 0) {
            while (list.firstChild) list.removeChild(list.firstChild);
        }

        for (var element in data){
            createListItem(data[element].nome, data[element].cognome, data[element].id);
            let newUser = { 
                'nome' : data[element].nome,
                "cognome" : data[element].cognome,
                "eta" : data[element].eta,
                "task" : data[element].task,
                "info" : data[element].info,
                "status" : data[element].status,
                "id" : data[element].id,
                "mac" : data[element].mobile_mac
            }
            users.push(newUser);
        }
        console.log(users);
      })
      .catch(error => {
          console.log('Error:', error);
      });
}

var users = [];


let interactiveSearch = document.querySelector('.search > input');
interactiveSearch.addEventListener('input', function(){
    let value = interactiveSearch.value;
    let listItems = document.querySelectorAll('.listItem');
    listItems.forEach(function(item){
        if(item.querySelector('.name').innerHTML.toLowerCase().includes(value.toLowerCase())){
            item.style.display = "flex";
        }else{
            item.style.display = "none";
        }
    });
});


var zoomedTunnelLen;
var multiplier = 0;
var i = 0;
var sens = 0;
var zoomCount = 1;
var sectionToDisplay = 0; 
var precZoom = [];
var nextZoom = {
    "tLen" : 0,
    "segLen" : 0,
    "sections" : 0,
    "multiplier" : 0
};

function unlockZoom(){
    document.querySelector('.zoomIn').style.opacity = "1";
    document.querySelector('.zoomIn').classList.add('hoverable');
    document.querySelector('.zoomOut').style.opacity = "1";
    document.querySelector('.zoomOut').classList.add('hoverable');
}

function zoomButtonHandler(zoom){
    createMap(zoom.sections, zoom.segLen);
    
}
document.querySelector('.zoomIn').addEventListener('click', function(){
    console.log(nextZoom);
    let actualSegLen = document.querySelector('.segLen').innerHTML;
    actualSegLen = actualSegLen.substring(0, actualSegLen.length - 1);
    let preczoom = {
        "tLen" : 0,
        "segLen" : 0,
        "sections" : 0,
        "multiplier" : 0
    };
    preczoom.multiplier = multiplier;
    preczoom.tLen = actualSegLen * 8;
    preczoom.segLen = actualSegLen;
    preczoom.sections = 8;
    console.log(preczoom);
    precZoom.push(preczoom);


    multiplier = nextZoom.multiplier;
    i = 0;
    zoomButtonHandler(nextZoom);
});

document.querySelector('.zoomOut').addEventListener('click', function(){
    //for (let el in precZoom) console.log(precZoom[el]);

    //offset = precZoom[precZoom.length - 1].multiplier;
    multiplier = precZoom[precZoom.length - 1].multiplier;
    sectionToDisplay = ((Number(precZoom[precZoom.length - 1].segLen)) * 8) / Number(anchorDistance);
    i = 0;
    zoomButtonHandler(precZoom[precZoom.length - 1]);
    precZoom.pop();
});


function zoomIntoMap(mapSection){
    unlockZoom();
    let preczoom = {
        "tLen" : 0,
        "segLen" : 0,
        "sections" : 0,
        "multiplier" : 0
    };
    zoomCount++;
    mapSection.classList.toggle('mapSection');
    mapSection.classList.toggle('mapSectionTemp');
    preczoom.multiplier = multiplier;
    multiplier = mapSection.classList[0];
    nextZoom.multiplier = multiplier;

    document.querySelectorAll('.mapSection').forEach(function(section){
        if(section != mapSection) section.remove();
    });

    setTimeout(function(){
        mapSection.classList.toggle('mapSectionTemp');
        mapSection.classList.toggle('mapSection');
        setTimeout(function(){
            document.querySelector('.mapSection').remove();
            i = 0;
            let actualSegLen = document.querySelector('.segLen').innerHTML;
            actualSegLen = actualSegLen.substring(0, actualSegLen.length - 1);
            
            zoomedTunnelLen = actualSegLen;

            sectionToDisplay = Math.ceil(Number(actualSegLen) / Number(anchorDistance));
  
            if (sectionToDisplay >= 8) {
                preczoom.tLen = actualSegLen * 8;
                preczoom.segLen = actualSegLen;
                preczoom.sections = 8;
                nextZoom.segStart = offset;
                nextZoom.segLen = Number(zoomedTunnelLen) / 8;
                nextZoom.sections = 8;
                console.log(nextZoom);

                precZoom.push(preczoom);
                
                createMap(8, Number(zoomedTunnelLen) / 8);

            }
            else {
                preczoom.tLen = actualSegLen * 8;
                preczoom.sections = 8;
                preczoom.segLen = actualSegLen;
                nextZoom.segStart = offset;
                nextZoom.segLen = Number(zoomedTunnelLen) / Number(sectionToDisplay);
                nextZoom.sections = sectionToDisplay;
                console.log(nextZoom);
                precZoom.push(preczoom);
                createMap(sectionToDisplay, Number(zoomedTunnelLen) / Number(sectionToDisplay));
            }
        
        }, 1000);
    }, 500);
}


function handleDots(anchor, id){
    for (var i in mapSections){
        if (mapSections[i].anchorL <= anchor && mapSections[i].anchorR >= anchor){
            if (mapSections[i].connectedId.includes(id) == false) {
                mapSections[i].connectedId.push(id);
                mapSections[i].dotNum++;
            }
        }
        else {
            if (mapSections[i].connectedId.includes(id)){
                mapSections[i].connectedId.splice(mapSections[i].connectedId.indexOf(id), 1);
                mapSections[i].dotNum--;
            }
        }
        
    }
    mapSections.forEach(function(section){
        if (section.dotNum > 0 && section.dotNum <= 5){
            let dots = section.section.querySelectorAll('.dot');
           
            if (section.dotNum == 1) dots[4].classList.add('visible');
            else if (section.dotNum == 2) dots[1].classList.add('visible');
            else if (section.dotNum == 3) dots[3].classList.add('visible');
            else if (section.dotNum == 4) dots[5].classList.add('visible');
            else if (section.dotNum == 5) dots[7].classList.add('visible');
            else {console.log()}
        }
    });
    
}
function mapGen(map, segNum, segLen){

    let mapSection = document.createElement('div');
    mapSection.classList.add('mapSection');

    let div = document.createElement('div');
    div.classList.add('dotDisplay');
    for (let i = 0; i < 9; i++){
        let dot = document.createElement('div');
        let shadow = document.createElement('div');
        dot.classList.add('dot');
        dot.classList.add( i );
        shadow.classList.add('dotShadow');
        shadow.classList.add( i );
        
        div.appendChild(dot);
        dot.appendChild(shadow);
        // setTimeout(function(){
        //     dot.classList.toggle('dot');
        //     dot.classList.toggle('biggerDot');
        //     let bigShadow = document.createElement('div');
        //     bigShadow.classList.add('biggerDotShadow');
        //     dot.appendChild(bigShadow);
        // }, 1000);
    }
    mapSection.appendChild(div);
    
    // il segmento che stiamo rappresentando diviso il numero di sottosegmenti
    mapSection.classList.add(i * (segLen) + offset);
        

    mapSection.style.height = "0px";
    //tunnelLen -= "m";  
    if (sectionToDisplay >= 16) mapSection.classList.add('hoverable');
    

   
    
    let placeholder = document.createElement('div');
    placeholder.classList.add('doubleAnchor');

    mapSection.appendChild(placeholder);

    let anchorL = document.createElement('div');
    anchorL.classList.add('anchor');
    anchorL.style.marginLeft = "-5px";

    let infos = document.createElement('div');
    infos.classList.add('anchorInfos');

    anchorL.addEventListener('mouseover', function(e){
        e.stopPropagation();
        infos.classList.add('hovered');
        console.log('hovered');
    });

    anchorL.addEventListener('mouseout', function(e){
        e.stopPropagation();
        infos.classList.remove('hovered');
    });

    anchorL.appendChild(infos);
    placeholder.appendChild(anchorL);

    infos.innerHTML = Math.ceil((segLen * i + offset) / anchorDistance);
    anchorL.style.opacity = "0";
    
    let anchorR = document.createElement('div');
    anchorR.classList.add('anchor');
    anchorR.style.marginRight = "-5px";
    placeholder.appendChild(anchorR);

    anchorR.style.opacity = "0";
    
    anchors.push(anchorL);
    anchors.push(anchorR);

    let mapSectionsItem = {
        "id" : i+1,
        "section" : mapSection,
        "anchorL" : Number(infos.innerHTML),
        "anchorR" : Math.ceil((segLen * (i+1) + offset) / anchorDistance),
        "dotNum" : 0,
        "connectedId" : []

    }
    
    if (i == 0){
        anchorL.classList.remove('anchor');
        anchorL.classList.add('firstAnchor');
        infos.innerHTML = Math.ceil(offset / anchorDistance);
        mapSectionsItem.anchorL = Math.ceil(offset / anchorDistance);
    }
    if (i == segNum - 1){
        anchorR.classList.remove('anchor');
        anchorR.classList.add('lastAnchor');
        let infos = document.createElement('div');
        infos.classList.add('anchorInfosLast');
        if (segLen * segNum == tunnelLen) infos.innerHTML = Math.ceil((segLen * (i+1) + offset) / anchorDistance) + 1;
        else infos.innerHTML = Math.ceil((segLen * (i+1) + offset) / anchorDistance);
        anchorR.appendChild(infos);

        anchorR.addEventListener('mouseover', function(e){
            e.stopPropagation();
            infos.classList.add('hovered');
            console.log('hovered');
        });
    
        anchorR.addEventListener('mouseout', function(e){
            e.stopPropagation();
            infos.classList.remove('hovered');
        });

        mapSectionsItem.anchorR = Number(infos.innerHTML);
    }
    i++;
    console.log(mapSections);
    mapSections.push(mapSectionsItem);
        
    
    // se 8 no zoom 16 zoom 24 2 zoom 
    if (sectionToDisplay >= 16) {
        mapSection.addEventListener('click', function(e){
            e.stopPropagation();
            zoomIntoMap(mapSection);
        });
    }

    map.appendChild(mapSection);
        if (i < segNum) mapGen(map, segNum, segLen);
        else {mapDraw();}
    }

function mapDraw(){
    setTimeout(function(){
        anchors.forEach(function(anchor){
            anchor.style.opacity = "1";
        });
    }, 100);

    setTimeout(function(){ 
        mapSections.forEach(function(section){
            section.section.style.height = "100%";
        });
    }, 500);

}


var mapSections = [];
var anchors = [];


let confirmMapInfo = document.querySelector('.submitMapInfo');
var tunnelLen;
var anchorsNum;

var minAnchorNum;
var anchorDistance = 0;

document.querySelector('input[name = "tunnelLen"]').addEventListener('keyup', function(){
    tunnelLen = document.querySelector('input[name = "tunnelLen"]').value;
    if (tunnelLen == 0 || tunnelLen == "") minAnchorNum = "";
    
    minAnchorNum =  Math.ceil(tunnelLen / 8);
    if (tunnelLen <= 8) {
        document.querySelector(".anchorMin").innerHTML = 1;
        document.querySelector('input[name = "anchorsNum"]').value = 1;
    }
    else {
        minAnchorNum--;
        document.querySelector(".anchorMin").innerHTML = minAnchorNum;
        document.querySelector('input[name = "anchorsNum"]').value = minAnchorNum;
    }

    let dist = tunnelLen / (minAnchorNum+1);
    dist = dist.toFixed(2);
    if (dist > 8) dist = 8;
    document.querySelector(".anchorDistance").innerHTML = dist;
    document.querySelector(".submitMapInfo").style.backgroundColor = "rgba(0, 98, 255, 0.8)";
});

document.querySelector('input[name = "anchorsNum"]').addEventListener('keyup', function(e){
    anchorsNum = document.querySelector('input[name = "anchorsNum"]').value;
    
    anchorDistance = Number(tunnelLen) / ((Number(anchorsNum) + 1));
    document.querySelector(".anchorDistance").innerHTML = anchorDistance.toFixed(3);
    if (anchorDistance > 8  || anchorsNum < minAnchorNum) {
        document.querySelector(".submitMapInfo").style.backgroundColor = "rgba(255, 0, 115, 0.8)";
    }
    else {
        document.querySelector(".submitMapInfo").style.backgroundColor = "rgba(0, 98, 255, 0.8)";
    }
    //background-color: rgba(0, 98, 255, 0.8);
    //background-color: rgba(255, 0, 115, 0.8);
    });

let segmentLen = 0;
confirmMapInfo.addEventListener('click', function(){
    tunnelLen = document.querySelector('input[name = "tunnelLen"]').value;
    anchorsNum = document.querySelector('input[name = "anchorsNum"]').value;

    anchorDistance = Number(tunnelLen) / (Number(anchorsNum));
    sectionToDisplay = Math.ceil(Number(tunnelLen) / Number(anchorDistance));
    /*
    sens = Number(tunnelLen) / (Number(anchorsNum) + 1);
    segmentLen = sens;
    */

    if (confirmMapInfo.style.backgroundColor == "rgba(255, 0, 115, 0.8)"){
        confirmMapInfo.classList.add('shake');
        setTimeout(function(){
            confirmMapInfo.classList.remove('shake');
        }, 500);
    }
    else if (tunnelLen == "" || anchorsNum == ""){
        confirmMapInfo.classList.add('shake');
        setTimeout(function(){
            confirmMapInfo.classList.remove('shake');
        }, 500);
    }
    else{

        let segNum = anchorsNum;
        segNum++; //2

        zoomedTunnelLen = tunnelLen;
        if (segNum > 8){
            createMap(8, Math.ceil(tunnelLen/8));
        }
        else {
            createMap(segNum, Math.ceil(tunnelLen/segNum)); 
        }
    }
       
});
var offset = 0;
function createMap(segNum, segLen){
    
    if (document.querySelector('.form') != null)
        document.querySelector('.form').style.opacity = "0";

    setTimeout(function(){ 
        if (document.querySelector('.form') != null)
            document.querySelector('.form').remove();
        if (document.querySelector('.tunnelLenghtVisualizer') != null) {
            document.querySelector('.tunnelLenghtVisualizer').remove();
            document.querySelector('.map').remove();
        }
        let tunnelLenghtVisualizer = document.createElement('div');
        tunnelLenghtVisualizer.classList.add('tunnelLenghtVisualizer');
        document.querySelector('.footerCompressed')?.appendChild(tunnelLenghtVisualizer);
        document.querySelector('.footerExtended')?.appendChild(tunnelLenghtVisualizer);

        for (let i = 0; i < segNum; i++){
            
            let segment = document.createElement('div');
            segment.classList.add('segment');
            
            let anchorDisplayL = document.createElement('div');
            anchorDisplayL.classList.add('anchorDisplay');
            offset = Number(multiplier);
            let meters = ( Number(segLen) * Number(i) ) + (Number(multiplier));
            let tLen = 0;
            meters = meters.toPrecision(3);
            if ( i == segNum - 1) tLen = Number(meters.split("m")[0]) + Number(segLen); 
            if (meters >= 1000) meters = meters / 1000 + "km";
            else meters += "m";
            anchorDisplayL.innerHTML = meters;

            segment.appendChild(anchorDisplayL);

            
            if ( i == segNum - 1){ 
                let anchorDisplayR = document.createElement('div');
                anchorDisplayR.classList.add('anchorDisplay');
                tLen = Math.ceil(tLen);
                if (tLen >= 1000) {
                    tLen = tLen.toPrecision(4);
                    tLen = tLen / 1000 + "km";
                    
                }
                else {
                    tLen = tLen.toPrecision(3);
                    tLen = tLen + "m";
                    
                }
                anchorDisplayR.innerHTML = tLen;
                anchorDisplayR.style.marginRight = "-20px";
                segment.appendChild(anchorDisplayR);

                
            }
            
            

            tunnelLenghtVisualizer.appendChild(segment);
            let zoom = document.querySelector('.zoom');
            zoom.style.opacity = "1";
            let segLenDisplay = document.querySelector('.segLen');
            segLenDisplay.innerHTML = segLen + "m";
        }

        let firstMap = document.createElement('div');
        firstMap.classList.add('map');
        document.querySelector('.footerCompressed')?.appendChild(firstMap);
        document.querySelector('.footerExtended')?.appendChild(firstMap);
        firstMap.style.opacity = "0";
        setTimeout(function(){
            firstMap.style.opacity = "1";
        }, 100);
        setTimeout(function(){
            mapGen(firstMap, segNum, segLen);
        }, 1000);
    }, 500);
}



/*
Anum >= minAnum => Adist <= 8

ceil(Tlen / Anum) = Adist

ceil(Tlen / Adist) = mapSegLen

ceil(mapSegLen / Adist) = Ancheron to display


Tlen = 1000m
Anum = 139

Adist = 1000 / 139 = 7.19 => 8
mapSegLen  = 1000 / 8 = 125

Ancheron to display = 125 / 8 = 15.625 => 16


*/

const socket = new WebSocket('ws://' + ip + '/get_all');

socket.onopen = function(e) {
    console.log("Connection established");
    socket.send(["Hello"]);
    console.log("[open] ciao");
};
var datas = [];
socket.onmessage = function(event) {
    console.log(`[message] Data received from server: ${event.data}`);
    let data = JSON.parse(event.data);
    let id = data["id"];
    let mac = data["mac_dispositivo"];
    let sens = data["sensor_data"];
    let anchor = data["sensor_id"]
    console.log(anchor);
    sens = sens.split("{")[1];
    sens = sens.split("}")[0];
    sens = sens.split(",");

    let newData = {
        "id" : id,
        "mac" : mac,
        "temp" : sens[0],
        "sat" : sens[1],
        "hb" : sens[2],
        "col" : sens[3],
        "sugar" : sens[4],
        "dead" : sens[5],
        "anchor" : anchor
    } 

    if (datas.length == 0) datas.push(newData);
    else {
        for (var i in datas){
            if (datas[i].id == id && datas[i].anchor == anchor){
                datas[i] = newData;
            }
            else if(datas[i].id == id && datas[i].anchor != anchor ){
                datas.push(newData);
               
            }

            else if (i == datas.length - 1){
                datas.push(newData);
            }
        }
    }

    refreshStats();
    handleDots(anchor, id);
};

setInterval(function(){
    
}, 10000);

function refreshStats(){
    let card = document.querySelector('.card');
    let style = window.getComputedStyle(card);
    if (style.display != "none") {
        let index = Number(card.classList[1]);
        for (var i in datas) {
            if (datas[i].id == index) {
                data = datas[i];
            }
        }
        
        let tempVal = document.querySelector('.TPvalue');
        tempVal.innerHTML = data.temp;

        let hbVal = document.querySelector('.HRvalue');
        hbVal.innerHTML = data.hb;

        let satVal = document.querySelector('.O2value');
        satVal.innerHTML = data.sat;
    }
    
    
}

function putMacAddress(url, data) {
    fetch(url, {
        method: 'POST',
        body: JSON.stringify(data),
        headers: {
          'Content-Type': 'application/json'
        }
      })
      .then(response => response.json())
      .then(data => {
        console.log('status:', data.status);
        if (data.status == "ok") missionStarted();
        else missionStartError();
      })
      .catch(error => {
          console.log('Error:', error);
      });
}



