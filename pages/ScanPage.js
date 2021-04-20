import React, { useState, useEffect } from 'react';
import { Text, View, StyleSheet, Button } from 'react-native';
import { BarCodeScanner } from 'expo-barcode-scanner';
import Odoo from 'react-native-odoo-promise-based'
import { Card } from 'react-native-elements';


const odoo = new Odoo({
    host: '192.168.8.105',
    port: 8069, // Defaults to 80 if not specified 
    database: 'Plasmionique',
    username: 'alex.chang@plasmionique.com', // Optional if using a stored session_id 
    password: 'Alexyy0229!', // Optional if using a stored session_id 
    //sid: 'YOUR_SESSION_ID', // Optional if using username/password 
    //protocol: 'https' // Defaults to http if not specified 
})

const Scan = () => {
    const [hasPermission, setHasPermission] = useState(null);
    const [scanned, setScanned] = useState(false);
    const [item, setItem] = useState(null)

    const itemInfo = (dbResult) => {
        return (
            <View style={{ paddingBottom: 10 }}>
                {dbResult == null ?
                    (<Text>Loading...</Text>) :
                    (<Card>
                        <Card.Title>{dbResult.name}</Card.Title>
                        <Card.Divider />
                        <Text>Internal ID: {dbResult.internal_id}</Text>
                        <Text>Manufacturer ID:  {dbResult.mpn}</Text>
                        <Text>Quantity: {dbResult.qty_available}</Text>
                    </Card>)}
            </View>
        )
    }

    useEffect(() => {
        (async () => {
            const { status } = await BarCodeScanner.requestPermissionsAsync();
            setHasPermission(status === 'granted');
        })();
    }, []);

    const getItem = (scanCode) => {
        const param = {
            domain: [['internal_id', '=', scanCode]],
            fields: ['name', 'internal_id', 'mpn', 'id', 'qty_available'],
        }
        odoo.search_read('product.template', param)
            .then(response => {
                if (response.data[0] !== undefined) {
                    console.log(response.data[0].name)
                    setItem(response.data[0])
                }
                else {
                    setItem(null)
                    alert("Non-valid code!")
                }
            })
            .catch(e => console.log(e))
    }

    useEffect(() => {
        odoo.connect()
            .then(response => { console.log('connected!') })
            .catch(e => { console.log(e) })
    }, [])

    const handleBarCodeScanned = ({ type, data }) => {
        var scanCode = data
        setScanned(true);
        getItem(scanCode)
        alert(`Bar code with type ${type} and data ${data} has been scanned!`);
    };

    if (hasPermission === null) {
        return <Text>Requesting for camera permission</Text>;
    }
    if (hasPermission === false) {
        return <Text>No access to camera</Text>;
    }

    return (
        <View style={styles.container}>
            {scanned === false ? (
                <BarCodeScanner
                    type={(BarCodeScanner.Constants.Type.back)}
                    barCodeTypes={[BarCodeScanner.Constants.BarCodeType.qr]}
                    onBarCodeScanned={scanned ? undefined : handleBarCodeScanned}
                    style={StyleSheet.absoluteFillObject}
                />) : (<Text></Text>)}
            {scanned &&
                <View style={{ alignItems: "center", justifyContent: "center", }}>
                    {itemInfo(item)}
                    <Button title={'Tap to Scan Again'} onPress={() => setScanned(false)} />
                </View>
            }
        </View>
    )
}

export default Scan

const styles = StyleSheet.create({
    container: {
        backgroundColor: 'black',
        color: 'white',
        alignItems: 'center',
        flex: 1,
        flexDirection: 'column',
        justifyContent: 'center',
    },
});
