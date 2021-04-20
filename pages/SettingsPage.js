import React, { useState } from 'react'
import { View, Text, StyleSheet } from 'react-native'
import { AsyncStorage } from 'react-native';
import { TextInput } from 'react-native-paper';

const Settings = () => {
    const [OdooId, setOdooId] = useState('')
    const [OdooPass, setOdooPass] = useState('')

    return (
        <View style={styles.container}>
            <TextInput
                style={styles.input}
                label='Odoo ID'
                onChangeText={(text) => setOdooId(text)}
                value={OdooId}
            />
            <TextInput
                style={styles.input}
                label='Odoo password'
                onChangeText={(text) => setOdooPass(text)}
                value={OdooPass}
            />
        </View>
    )
}

export default Settings

const styles = StyleSheet.create({
    container: {
        backgroundColor: 'white',
        flex: 1,
        flexDirection: 'column',
        justifyContent: 'center',
    },
    input: {
        marginLeft: '4%',
        marginBottom: '2%',
        marginRight: '4%'
    },
});