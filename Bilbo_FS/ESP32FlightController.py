import requests

class ESP32FlightController:
    
    # Initialize parameters
    def __init__(self, ipAddress, port, ):
        self.ip = ipAddress
        self.ipPort = ipPort
        self.url = f'http://{ipAddress}:{ipPort}'
        self.serial_port = serial_port
        self.baudrate = baudrate
        self.IPconnections = []
        self.ser_connected = False  # Track connection status
        self.ap_connected = False  # Track connection status



    def add_APconnection(self, device):
        self.IPconnections.append(device)
    
    def get_connections(self):
        count = 0
        for device in self.connections:
            count = 1 + count
            print(device)
        return self.connections, "Number of connections: ", count
    
    def get_AP_connections(self):
        try:
            response = requests.get(self.url, timeout=2)
            response.raise_for_status()
            print(response.text)
            return response.text
        except requests.RequestException as e:
            message = " Unable to pair with FC"
            print(f"Unable to pair with FC: {e}")
            self.serial_connection = None  # No connection, but program still runs
        
    def open_serial_connection(self):
        try:
            self.serial_connection = serial.Serial(self.serial_port, self.baudrate, timeout=1)
            time.sleep(2)  # Allow ESP32 to initialize
            self.ser_connected = True  # Mark connection as successful
            print(f"Connected to ESP32 on {self.serial_port}")
        except serial.SerialException as e:
            print(f"Error: Could not open serial port {self.serial_port}. Running in simulation mode.")
            self.serial_connection = None  # No connection, but program still runs
    
    sensor_data = {"pressure": "--", "altitude": "--", "acceleration": "--", "gyroscope": "--"}  

    def read_serial(self):
        """Read UART and update web UI"""
        global sensor_data
        while True:
            try:
                line = self.serial_connection.readline().decode().strip()
                if line:
                    print(f"Received: {line}")
                    sensor_data = json.loads(line)  
                    socketio.emit("sensor_update", sensor_data)  
            except Exception as e:
                print(f"Error: {e}")

    def close_connection(self):
        # Close the serial connection
        if self.serial_connection:
            self.serial_connection.close()

    # def get_imu_data(self):
    #     return 
    
    # def get_altimeter_data(self):
    #     return
    
    # def get_ultrasound_data(self):
    #     return
        