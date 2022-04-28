const red_pin = "https://github.com/andreamazzitelli/checkBin/blob/main/img/pins/red-pin-small.png?raw=true";
const yellow_pin = "https://github.com/andreamazzitelli/checkBin/blob/main/img/pins/yellow-pin-small.png?raw=true";
const green_pin = "https://github.com/andreamazzitelli/checkBin/blob/main/img/pins/green-pin-small.png?raw=true";

async function addBin(lat, lng) {

    lat = lat.replace(/\s+/g, '')
    lng = lng.replace(/\s+/g, '')

    if (typeof (lat) == 'undefined' || typeof (lng) == 'undefined' || lat == null || lng == null || isNaN(lat) || isNaN(lng) || lat == "" || lng == "") {
        alert("Insert Correct Coordinates");
        return
    }

    var payload = {
        "lat": lat,
        "lng": lng
    }

    var param = {
        method: 'POST',
        headers: {
            'Accept': 'applicattion/json'
        },
        body: JSON.stringify(payload)
    }

    var result = await fetch("addBin Function URL", param)
        .then(data => { return data.json() })
        .then(data => {
            console.log(data)
            console.log(data['id'])
            var new_marker = [{
                'lat': {
                    'S': lat
                },
                'lng': {
                    'S': lng
                },
                'id': {
                    'S': data['id']
                },
                'last_fill_level': {
                    'N': '0'
                },
                'last_fill_timestamp': {
                    'S': parseInt(Math.round(Date.now() / 1000))
                }
            }]
            return new_marker
        })
        .then(data => {
            console.log(data)
            setMarkers(map, infoWindow, data);
            document.getElementById('lat').value = '';
            document.getElementById('lng').value = ''
            document.getElementById('show').innerHTML = 'Added Bin with ID: ' + data[0]['id']['S']
        })

}

function setMarkers(map, infoWindow, locations) {

    for (var i = 0; i < locations.length; i++) {

        var location = {
            lat: parseFloat(locations[i]['lat']['S']),
            lng: parseFloat(locations[i]['lng']['S'])
        }
        var date = new Date(parseInt(locations[i]['last_fill_timestamp']['S']) * 1000)
        var last_fill_level = locations[i]['last_fill_level']['N']
        var fill_level = parseFloat(locations[i]['last_fill_level']['N'])
        var id = "id: "+locations[i]['id']['S'] + ' - fill level: ' + last_fill_level + ' at ' + date.toLocaleString()

        var image;

        if (fill_level >= 8) {
            image = red_pin
        }
        else if (fill_level >= 5) {
            image = yellow_pin
        }
        else {
            image = green_pin
        }

        //console.log(location)
        const marker = new google.maps.Marker({
            position: location,
            map: map,
            title: id,
            icon: image,
            optimized: false,
        });

        marker.addListener("click", () => {
            infoWindow.close();
            console.log(marker.getTitle())
            infoWindow.setContent(marker.getTitle());
            infoWindow.open(marker.getMap(), marker);
        });
    }
}

// Initialize and add the map
async function initMap() {
    // The map, centered at the location
    const center_position = { lat: 41.914718666319914, lng: 12.523279060428838 };
    infoWindow = new google.maps.InfoWindow();
    map = new google.maps.Map(document.getElementById("map"), {
        zoom: 10,
        center: center_position,
    });

    var locations = []

    var result = await fetch("getBins Function URL")
        .then(data => { return data.json(); })
        .then(data => {
            data.forEach(el => {
                locations.push(el)
            })
        })

    console.log(locations)
    setMarkers(map, infoWindow, locations)
}
var map;
var infoWindow;
window.initMap = initMap;