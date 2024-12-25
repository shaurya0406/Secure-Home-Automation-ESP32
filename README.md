## NOTE: All the liscences and references are mentioned in their respective files.
##  You don't have to download any extra app.This project is compatible with Apple HomeKit App that comes pre-installed on your iOS Device. Support for google home and alexa will be added soon.

## Demo Video
[![DEMO VIDEO](https://img.youtube.com/vi/5DxWyh5Lduk/0.jpg)](https://www.youtube.com/watch?v=5DxWyh5Lduk)

## Introduction

Today there are millions of IoT devices but only a few address the most important issue of security and data-privacy. 
Anyone who has the server access be it a hacker, or the manufacturer or the service provider itself, can control your devices.
In the Mirai attack, only Apple HomeKit devices survived, so we have based our communication model on HomeKit.
If we want to connect more devices, we can dynamically add them to our home using the mesh network which primarily uses tree topology.
To implement a server less model, we have peer-to-peer(P2P) communication between the nodes of the mesh as well as with the smartphone.
To enable global access of our devices, the smartphone uploads the device status to iCloud with encryption enabled. At no time the devices are directly connected to the cloud server. The smartphone first receives the command and then send it to the device using either BLE or WiFi.

All of these features can be easily implemented on ESP32 dual core MCU with supporting Ultra Low Power(ULP) cores and hardware acceleration for ML tasks( explained later).

## How our implementation is better than the existing market players:
### **SECURITY**
We have implemented encryption not only at software level but also in our hardware(ESP32) by customising the partition scheme.To implement a **Server-Less** model, we have **Peer-to-Peer(P2P)** communication between the nodes of the mesh as well as with the smartphone.To enable global access of our devices, the smartphone uploads the device status to iCloud with encryption enabled. At no time the devices are directly connected to the cloud server. .So no one can control our devices until they have gone through the initial setup process wihich only involves scanning of QR-code from smart-phone and all the wifi and cloud credential are automatically configured. 
At the **Hardware** level, it is important to enable on device secure encryption boot and flash of the firmware.This removes the possibility of hacking into the system manually by reading the Flash memory registers of the device(ESP32).For this we have made a custom NVS partition scheme to enable code operation modularity:

- 2 OTA update partitions for secure boot after over-the-air update of firmware.
- HomeKit and MESH running on 1 core and Wake word detection on the other.(ESP32 is a dual core SoC)

### Dynamic Addition of various types of devices with just 1 product using MESH which communicates on our custom encrypted networking protocol rather than HTTPS or MQTT.

### Our device does'nt require the users to download any extra app.Our device works and syncs with both **Apple-HomeKit**, **Google Home** apps which come **Pre-Installed** on all iOS and Android devices.

### **On Device Offline Speech Recognition which will be our source of revenue**

### **Our Product is easily scalable and can be implemented on a very large agricultural field or an industrial factory for automation without making any changes**

###**Our product is fault-tolerant with priority levels given to the device which was initially used for setup and is maintained with our mesh network.**


### Structure of Appliances: 

Device->Accessories->Services->Characteristics

Our Device is a Bridge Accessory->with Humidity,Temperature,LightBulb,Fan etc, Services-> with current Humidity,Temperature,LightBulb_On_State,Brightness,Hue, Fan_On_State etc. Characteristics.

To add multiple devices, we have implemented a mesh.
Our MESH differs from traditional infrastructure Wi-Fi networks in that nodes are not required to connect to a central node. Instead, nodes are permitted to connect with neighboring nodes. Nodes are mutually responsible for relaying each others transmissions.This is also less susceptible to overloading as the number of nodes permitted on the network is no longer limited by a single central node.

With this we have built a system which can be easily scaled up to use in a large agricultural field or industrial factory with utmost security.


HomeKit is a framework developed by Apple for communicating with and controlling connected accessories in a user’s home using iOS devices. 

From the very start of device setup, the communication is entirely between the smart-phone and ESP32(Microcontroller of our project) over WiFi.

With HomeKit, we have enabled encrypted P2P communication and all the feature available with the framework like adding devices during run-time, giving access to other family members of the home using iCloud based key distribution.

We have also enabled sensor data(Temperature, Humidity etc.) based triggers which can be customised with the HomeKit app.

Support for Google Home and Amazon Alexa will be addded soon. They need pre-signed certificates to be exchanged between the smart-phone and the device for mutual authentication.


## MESH

Our MESH differs from traditional infrastructure Wi-Fi networks in that nodes are not required to connect to a central node. Instead, nodes are permitted to connect with neighboring nodes. Nodes are mutually responsible for relaying each others transmissions. This allows MESH network to have much greater coverage area as nodes can still achieve interconnectivity without needing to be in range of the central node. Likewise, this is also less susceptible to overloading as the number of nodes permitted on the network is no longer limited by a single central node.

Tree Topology

The MESH is built atop the infrastructure Wi-Fi protocol and can be thought of as a networking protocol that combines many individual Wi-Fi networks into a single WLAN. In Wi-Fi, stations are limited to a single connection with an AP (upstream connection) at any time, whilst an AP can be simultaneously connected to multiple stations (downstream connections). However it allows nodes to simultaneously act as a station and an AP. Therefore a node in MESH can have multiple downstream connections using its softAP interface, whilst simultaneously having a single upstream connection using its station interface. This naturally results in a tree network topology with a parent-child hierarchy consisting of multiple layers.

This is a multiple hop (multi-hop) network meaning nodes can transmit packets to other nodes in the network through one or more wireless hops. Therefore, nodes in our MESH not only transmit their own packets, but simultaneously serve as relays for other nodes. Provided that a path exists between any two nodes on the physical layer (via one or more wireless hops), any pair of nodes within the MESH network can communicate.

The size (total number of nodes) in the MESH network is dependent on the maximum number of layers permitted in the network, and the maximum number of downstream connections each node can have. Both of these variables can be configured to limit the size of the network.

Beacon Frames & RSSI Thresholding

Every node in the MESH that is able to form downstream connections (i.e. has a softAP interface) will periodically transmit Wi-Fi beacon frames. A node uses beacon frames to allow other nodes to detect its presence and know of its status. Idle nodes will listen for beacon frames to generate a list of potential parent nodes, one of which the idle node will form an upstream connection with. It uses the Vendor Information Element to store metadata such as:

- Node Type (Root, Intermediate Parent, Leaf, Idle)
- Current layer of Node
- Maximum number of layers permitted in the network
- Current number of child nodes
- Maximum number of downstream connections to accept

The signal strength of a potential upstream connection is represented by RSSI (Received Signal Strength Indication) of the beacon frames of the potential parent node. To prevent nodes from forming a weak upstream connection, MESH implements an RSSI threshold mechanism for beacon frames. If a node detects a beacon frame with an RSSI below a preconfigured threshold, the transmitting node will be disregarded when forming an upstream connection.

### Communication Protocol
**Our product is not using HTTPS or MQTT. Instead we have implemented a whole new concept.**

Every WiFi modem along with its RSSI Values, broadcasts **Sub Channel State (CSI) Values** for MAC address etc. This also consumes a lot of power so why not make good use of it. We will broadcast our encrypted data throughout the mesh in a compressed JSON script. All the other nodes will just capture this data by tuning into the fundamental frequency of the radio waves and will update their status.

**With this simple concept we have also enabled fault-tolerance as each device acts as a backup for the other.**

## Challenges we ran into:

The first challenge was to implement the mesh network.

ESP32 offers 28 sub channel state information using which we communicate with the other nodes.
Setting the right channel info was a very tough challenge which we solved by calibrating each channel using the other, hence we were able to filter out the off-frequency waves and got the fundamental one.

Another challenge was to enable local control to minimise delay when on same wifi-network. this was solved by deirectly relaying the messages via the wifi router if using wifi for communication else BLE does'nt have this problem.


Sometimes it may happen that your smartphone is not in the same room as your devices and you want to control them using voice commands( Otherwise voice control is enabled using Siri/Google Asst.).
For that we need speech recognition sytem. How it is done in Alexa: They continuously send the audio captured to their server for processing. So what we did is that we enabled the wake word detection (Alexa or Marvin in our project) on the ESP32 itself.

If and only if the wake word is detected, then the communication with external server(Wit.ai) starts for intent detection.
Even if someone wants to control the devices directly from the server, he will not be able to do it until the wake word was detected.
Hence secured from both ends.


For wake word detection, LSTM iis better than CNN as for less input it gives better results. It can create its own dataset if there is any error during run time.
But we could only find support for CNN in TensorFlow Lite for embedded MCUs, which enables the conversion of the trained Neural Network to C++ code.


We have also enabled OTA updates and again for secure flash and boot we have made our own partition scheme for our Non-Volatile Flash File System.

On one Core of ESP32 we run the wake word detection and on the other we run the Homekit as well as Mesh network.
One of the cool features of our project is that as this is based on stock Apple and Google Home apps, we have direct integration with their other services fully optimised to work on them.

## Speech Recognition for Voice Control

To enable Voice control, instead of sending all the captured audio continuously to the server, we have a wake word detection system based on a CNN neural network which detects the word "MARVIN" and then send the audio to the server for further processing.

Without this wake word there is zero communication with the server and any command sent from the server will not be received until the wake word is detected.
We have also added a stop button in the LCD interface of the device so as to save power when there is no one in the room.

We need to build three components:

- Wake word detection
- Audio capture and Intent Recognition
- Intent Execution

## Wake Word Detection

We need to create something that will tell us when a "wake" word is heard by the system. This will need to run on our embedded devices - an ideal option for this is to use TensorFlow and TensorFlow Lite.

### Training Data

Our first port of call is to find some data to train a model against. We can use the [Speech Commands Dataset](https://www.tensorflow.org/datasets/catalog/speech_commands). This dataset contains over 100,000 audio files consisting of a set of 20 core commands words such as "Up", "Down", "Yes", "No" and a set of extra words. Each of the samples is 1 second long.

One of these words in particular looks like a good candidate for a wake word - We have chosen to use the word "Marvin"

To augment the dataset we have also record ambient background noise, several hours of household noises and TV shows to provide a large amount of random data.

A popular approach for word recognition is to translate the problem into one of image recognition.

We need to turn our audio samples into something that looks like an image - to do this we can take a spectrogram.

To get a spectrogram of an audio sample we break the sample into small sections and then perform a discrete Fourier transform on each section. This will give us the frequencies that are present in that slice of audio.

Putting these frequency slices together gives us the spectrogram of the sample.

In the `model` folder you'll find several Jupyter notebooks. Follow the setup instructions in the `README.md` to configure your local environment.

The notebook `Generate Training Data.ipynb` contains the code required to extract our features from our audio data.

The following function can be used to generate a spectrogram from an audio sample:

```python
def get_spectrogram(audio):
    # normalise the audio
    audio = audio - np.mean(audio)
    audio = audio / np.max(np.abs(audio))
    # create the spectrogram
    spectrogram = audio_ops.audio_spectrogram(audio,
                                              window_size=320,
                                              stride=160,
                                              magnitude_squared=True).numpy()
    # reduce the number of frequency bins in our spectrogram to a more sensible level
    spectrogram = tf.nn.pool(
        input=tf.expand_dims(spectrogram, -1),
        window_shape=[1, 6],
        strides=[1, 6],
        pooling_type='AVG',
        padding='SAME')
    spectrogram = tf.squeeze(spectrogram, axis=0)
    spectrogram = np.log10(spectrogram + 1e-6)
    return spectrogram
```

This function first normalises the audio sample to remove any variance in volume in our samples. It then computes the spectrogram - there is quite a lot of data in the spectrogram so we reduce this by applying average pooling.

We finally take the log of the spectrogram so that we don't feed extreme values into our neural network which might make it harder to train.

Before generating the spectrogram we add some random noise and variance to our sample. We randomly shift the audio sample the 1-second segment - this makes sure that our neural network generalises around the audio position.

```python
# randomly reposition the audio in the sample
voice_start, voice_end = get_voice_position(audio, NOISE_FLOOR)
end_gap=len(audio) - voice_end
random_offset = np.random.uniform(0, voice_start+end_gap)
audio = np.roll(audio,-random_offset+end_gap)
```

We also add in a random sample of background noise. This helps our neural network work out the unique features of our target word and ignore background noise.

```python
# get the background noise files
background_files = get_files('_background_noise_')
background_file = np.random.choice(background_files)
background_tensor = tfio.audio.AudioIOTensor(background_file)
background_start = np.random.randint(0, len(background_tensor) - 16000)
# normalise the background noise
background = tf.cast(background_tensor[background_start:background_start+16000], tf.float32)
background = background - np.mean(background)
background = background / np.max(np.abs(background))
# mix the audio with the scaled background
audio = audio + background_volume * background
```

To make sure we have a balanced dataset we add more samples of the word "Marvin" to our dataset. This also helps our neural network generalise as there will be multiple samples of the word with different background noises and in different positions in the 1-second sample.

```python
# process all the words and all the files
for word in tqdm(words, desc="Processing words"):
    if '_' not in word:
        # add more examples of marvin to balance our training set
        repeat = 70 if word == 'marvin' else 1
        process_word(word, repeat=repeat)
```

We then add in samples from our background noise, we run through each background noise file and chop it into 1-second samples, compute the spectrogram, and add these to our negative examples.

With all of this data we end up with a reasonably sized training, validation and testing dataset.

### Model Training

In the `model` folder you'll find another Jupyter notebook `Train Model.ipynb`. This takes the training, test and validation data that we generated in the previous step.

For our system we only really care about detecting the word Marvin so we'll modify our Y labels so that it is a 1 for Marvin and 0 for everything else.

```python
Y_train = [1 if y == words.index('marvin') else 0 for y in Y_train_cats]
Y_validate = [1 if y == words.index('marvin') else 0 for y in Y_validate_cats]
Y_test = [1 if y == words.index('marvin') else 0 for y in Y_test_cats]
```

We feed this raw data into TensorFlow datasets - we set up our training data repeat forever, randomly shuffle, and to come out in batches.

```python
# create the datasets for training
batch_size = 30

train_dataset = Dataset.from_tensor_slices(
    (X_train, Y_train)
).repeat(
    count=-1
).shuffle(
    len(X_train)
).batch(
    batch_size
)

validation_dataset = Dataset.from_tensor_slices((X_validate, Y_validate)).batch(X_validate.shape[0])

test_dataset = Dataset.from_tensor_slices((X_test, Y_test)).batch(len(X_test))
```
We have a convolution layer, followed by a max-pooling layer, following by another convolution layer and max-pooling layer. The result of this is fed into a densely connected layer and finally to our output neuron.

```python
model = Sequential([
    Conv2D(4, 3,
           padding='same',
           activation='relu',
           kernel_regularizer=regularizers.l2(0.001),
           name='conv_layer1',
           input_shape=(IMG_WIDTH, IMG_HEIGHT, 1)),
    MaxPooling2D(name='max_pooling1', pool_size=(2,2)),
    Conv2D(4, 3,
           padding='same',
           activation='relu',
           kernel_regularizer=regularizers.l2(0.001),
           name='conv_layer2'),
    MaxPooling2D(name='max_pooling2', pool_size=(2,2)),
    Flatten(),
    Dropout(0.2),
    Dense(
        40,
        activation='relu',
        kernel_regularizer=regularizers.l2(0.001),
        name='hidden_layer1'
    ),
    Dense(
        1,
        activation='sigmoid',
        kernel_regularizer=regularizers.l2(0.001),
        name='output'
    )
])
model.summary()
```

When I train this model against the data I get the following accuracy:

| Dataset            | Accuracy |
| ------------------ | -------- |
| Training Dataset   | 0.9683   |
| Validation Dataset | 0.9567   |
| Test Dataset       | 0.9562   |

These are pretty good results for such a simple model.

If we look at the confusion matrix using the high threshold (0.9) for the true class we see that we have very few examples of background noise being classified as a "Marvin" and quite a few "Marvin"s being classified as background noise.

|        | Predicted Noise | Predicted Marvin |
| ------ | --------------- | ---------------- |
| Noise  | 13980           | 63               |
| Marvin | 1616            | 11054            |

This is ideal for our use case as we don't want the device waking up randomly.

### Converting the model to TensorFlow Lite

With our model trained we now need to convert it for use in TensorFlow Lite. This conversion process takes our full model and turns it into a much more compact version that can be run efficiently on our micro-controller.

In the `model` folder there is another workbook `Convert Trained Model To TFLite.ipynb`.

This notebook passes our trained model through the `TFLiteConverter` along with examples of input data. Providing the sample input data lets the converter quantise our model accurately.

Once the model has been converted we can run a command-line tool to generate C code that we can compile into our project.

```
xxd -i converted_model.tflite > model_data.cc
```

---

## Intent Recognition

With our wake word detection model complete we now need to move onto something that can understand what the user is asking us to do.

For this, we will use the [Wit.ai](https://wit.ai) service from Facebook. This service will "Turn What Your Users Say Into Actions".

## ESP32 Hardware Components

Why we chose the ESP32?

Reason #1 – It is inexpensive

 The ESP32 modules are so inexpensive compared to many other connectivity solutions that are available on the market today. For example, if you take a look at the ESP32 modules that available on Digikeys website, you’ll find that these modules range from $3.80 to about $4.80 on average! These aren’t volume prices, but simply the cost to purchase modules one at a time.

Reason #2 – It’s a Bluetooth / Wi-Fi combo module

When it comes to the IoT, there is a definite need to be able to support multiple connectivity solutions. In some instances, Bluetooth may be preferred for local connectivity needs while a Wi-Fi connection is necessary for cloud and remote interactions with devices. When a single module can provide both solutions it’s definitely a win-win not just for the developer but also for the end user. If a product only requires a Wi-Fi connection, the Bluetooth stack can be disabled, or the Bluetooth can be left included as a feature to be added later. I’ve seen several clients who selected the ESP32 for its Wi-Fi features and then later enabled the Bluetooth in order to expand the feature set for their products.
BLE can also be used for our MESH Topology to reduce power consumption.

Reason #3 – Options available with additional Flash and pSRAM

We have made a custom partition scheme to enable code operation modularity.

. 2 OTA update partitions for secure boot.
. HomeKit and MESH running on 1 core and Wake word detection on the other.(ESP32 is a dual core SoC)


## The end user prototype

This contains: 
. BME280 I2C based temerature, humidity and Pressure sensor.
. Actuators and LEDs for device accessory control.
. LCD display with Touch Interface for Manual Control.
. ESP32 MCU based Dev Board.
. I2S Microphone
. I2S speaker controller // not added yet.

## USES

Our IoT Model is easily scalable, thanks to the mesh network and can also be used in a Factory or a Farm to collect sensor data. As we are able to deploy a neural network on the ESP32, we can also process the sensor data on the device itself and make predictions without ever communicating with external server.
It will take time but within a month of operation we will have enough data to be processed.


## Future Enhancements

While testing we came across an interesting concept of sensor-less motion and geature detection based on the Channel State Information (CSI) values. ML can be used to detect the variations in the reflected radio waves between the 2 nodes as there is some motion in the room and appliances can be controlled accordingly.
This can be achieved with already existing model with a future firmware update.

We will also add current meaduring hall effect sensor to report power consumption and  automatically control the accessories accordingly.
