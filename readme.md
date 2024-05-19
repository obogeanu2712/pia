Implementation of API on ESP32 module

The program first scans and creates a list of available WiFi networks and accept the network password (if applicable) specified by a mobile app. After the WiFi connection is established, the app requests a list of data. From the list, the user is be able to request details about a specific entry in the data list, acording to the API specifications.

Here's a list of the API requests and response specifications:

getNetworks

The getNetworks request is the first request sent by the app and is used for obtaining the list of available WiFi networks available to the ESP32 device, as well as setting the teamId parameter, which will be a required parameter for all responses.

{
    action: 'getNetworks',
    teamId: string
}

The teamId value must be saved in the ESP32 program. Without it, the app will not allow further actions. The expected response is a JSON encoded object for each WiFi network, in the following form:

{
    ssid: string,
    strength: int|float,
    encryption: string|int,
    teamId: string
}

Any additional fields will be ignored by the app.

connect

The connect request is used to connect to a specified network with the WiFi password (if required). Notice that the teamId parameter is no longer sent in this request or any other subsequent request, but the previously stored value will have to be included in all responses, matching the initial value the was sent by the getNetworks request

{
    action: 'connect',
    ssid: string,
    password: string
}

The expected response is a JSON encoded object in the following form:

{
    ssid: string,
    connected: bool,
    teamId: string
}

Any additional fields is be ignored by the app.

getData

The getData request is used for querying the list of records from the API.

{
    action: 'getData'
}

Each API returns a different set of attributes. The ESP32 program should send only the relevant response to the app. The required response is a JSON encoded object for each list record, in the following form:

{
    id: int|string,
    name: string,
    image: string (url|base64),
    teamId: string
}

Any additional fields will be ignored by the app.

getDetails

The getDetails request is used for querying a single record. The id attribute represents the id of the item from the previous list.

{
    action: 'getDetails',
    id: int|string
}

Since each API returns different attributes for the records, the description attribute must be constructed in the ESP32 program, using the relevant attributes returned by the API. The required response is a JSON encoded object in the following form:

{
    id: int|string,
    name: string,
    image: string (url|base64),
    description: string,
    teamId: string
}

Any additional fields will be ignored by the app.
