import React, { useState } from 'react'
import { View, Text, StyleSheet } from 'react-native'
import { SearchBar } from 'react-native-elements'

const SearchItems = () => {
    const [searchIn, setSearchIn] = useState('')
    return (
        <View style={styles.container}>
            <SearchBar
                round
                searchIcon={{ size: 24 }}
                lightTheme={true}
                placeholder="Search Inventory"
                onChangeText={(text) => setSearchIn(text)}
                value={searchIn}
            />
        </View>
    )
}

export default SearchItems

const styles = StyleSheet.create({
    container: {
        backgroundColor: 'white',
        flex: 1,
        paddingTop: '10%'
    },
});

