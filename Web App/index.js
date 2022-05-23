//static images for markers
const red_pin = "https://github.com/andreamazzitelli/checkBin/blob/main/img/pins/red-pin-small.png?raw=true";
const yellow_pin = "https://github.com/andreamazzitelli/checkBin/blob/main/img/pins/yellow-pin-small.png?raw=true";
const green_pin = "https://github.com/andreamazzitelli/checkBin/blob/main/img/pins/green-pin-small.png?raw=true";

//add bin to table with specified dev_eui and coordinates
async function addBin(dev_eui, lat, lng) {

    dev_eui = dev_eui.replace(/\s+/g, '')
    lat = lat.replace(/\s+/g, '')
    lng = lng.replace(/\s+/g, '')

    if (typeof (dev_eui) == 'undefined' || typeof (lat) == 'undefined' || typeof (lng) == 'undefined' || dev_eui == null || lat == null || lng == null || isNaN(lat) || isNaN(lng) || dev_eui == "" || lat == "" || lng == "") {
        alert("Insert Correct Coordinates");
        return
    }

    var payload = {
        "dev_eui": dev_eui,
        "lat": lat,
        "lng": lng
    }

    var param = {
        method: 'POST',
        headers: {
            'Accept': 'application/json'
        },
        body: JSON.stringify(payload)
    }

    var result = await fetch("https://leqwxnlmrn2cr6rylzct7bkuha0qfsgf.lambda-url.us-east-1.on.aws/", param)
        .then(data => {return data })
        .then(data => {

            if (data['status'] == "400") {
                alert("DevEUI already in use");
                return
            }
            return data.json()
        })
        .then(data => {

            if (typeof (data) == 'undefined') {
                return
            }

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
            //console.log(data)

            if (typeof (data) == 'undefined') {
                return
            }
            setMarkers(map, infoWindow, data);
            document.getElementById('dev_eui').value = '';
            document.getElementById('lat').value = '';
            document.getElementById('lng').value = '';
            document.getElementById('show').innerHTML = 'Added Bin with ID: ' + data[0]['id']['S']
        })

}

//set marker on the map
function setMarkers(map, infoWindow, locations) {

    for (var i = 0; i < locations.length; i++) {

        var location = {
            lat: parseFloat(locations[i]['lat']['S']),
            lng: parseFloat(locations[i]['lng']['S'])
        }

        var date = new Date(parseInt(locations[i]['last_fill_timestamp']['S'])*1000)
        var last_fill_level = locations[i]['last_fill_level']['N']
        var fill_level = parseFloat(locations[i]['last_fill_level']['N'])
        var id = "id: " + locations[i]['id']['S'] + ' - fill level: ' + last_fill_level + ' - at: ' + date.toLocaleString()

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
            //console.log(marker.getTitle())
            infoWindow.setContent(marker.getTitle());
            infoWindow.open(marker.getMap(), marker);
        });

        markers.push(marker)
    }


}

//function to delete bin given it's id
async function deleteBin(id) {

    id = id.replace(/\s+/g, '')

    if (typeof (id) == 'undefined' || id == null || isNaN(id) || id == "") {
        alert("Insert Correct ID");
        return
    }

    var string = "id: " + id;

    var payload = {
        "id": id
    }

    var param = {
        method: 'POST',
        headers: {
            'Accept': 'application/json'
        },
        body: JSON.stringify(payload)
    }

    var result = await fetch("https://64m2pkb43ny2mhxsz6ygtmm3ge0nkpeg.lambda-url.us-east-1.on.aws/", param)
        .then(() => {
            for (var i = 0; i < markers.length; i++) {
                if (markers[i].title.search(string) != -1) {
                    markers[i].setMap(null);
                    markers.splice(markers.indexOf(markers[i]), 1)
                }
            }
        })
        .then(() => {
            document.getElementById('bin_id').value = ''
            document.getElementById('show2').innerHTML = 'Deleted Bin with ID: ' + id
        })

}


// Initialize and add the map
async function initMap() {
    // The map, centered at the location
    const center_position = { lat: 41.914718666319914, lng: 12.523279060428838 };
    markers = []
    infoWindow = new google.maps.InfoWindow();
    map = new google.maps.Map(document.getElementById("map"), {
        zoom: 10,
        center: center_position,
        styles: [   //needed to remove the points of interests 
            {
                featureType: "poi",
                elementType: "labels",
                stylers: [{ visibility: "off" }]
            }
        ]
    });

    var locations = []

    var result = await fetch("https://slfosy6btrlq2u2xufldfoy5fi0reboc.lambda-url.us-east-1.on.aws/")
        .then(data => { return data.json(); })
        .then(data => {
            data.forEach(el => {
                locations.push(el)
            })
        })

    //console.log(locations)
    setMarkers(map, infoWindow, locations)

}
var map;
var markers = [];
var infoWindow;

//code to automatically reload the page, for now is commented to avoid to many calls
// setTimeout(function(){
//     window.location.reload(1);
// }, 10000)
