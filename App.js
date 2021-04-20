import React, { useState, useEffect } from 'react';
import { Text, View, StyleSheet, Button } from 'react-native';
import Odoo from 'react-native-odoo-promise-based'
import { NavigationContainer } from '@react-navigation/native';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';
import Scan from './pages/ScanPage';
import { Ionicons } from '@expo/vector-icons';
import SearchItems from './pages/SearchPage';
import Settings from './pages/SettingsPage';

const Tab = createBottomTabNavigator();

export default function App() {
  return (
    <NavigationContainer>
      <Tab.Navigator
        screenOptions={({ route }) => ({
          tabBarIcon: ({ focused, color, size }) => {
            let iconName;

            if (route.name === 'Scan') {
              iconName = focused
                ? 'scan'
                : 'scan-outline';
            } else if (route.name === 'Search') {
              iconName = focused
                ? 'search'
                : 'search-outline';
            } else if (route.name === 'Settings') {
              iconName = focused
                ? 'settings'
                : 'settings-outline';
            }
            return <Ionicons name={iconName} size={size} color={color} />;
          },
        })}
        tabBarOptions={{
          activeTintColor: 'tomato',
          inactiveTintColor: 'gray',
          style: { height: '7%' },
          keyboardHidesTabBar: true
        }}>
        <Tab.Screen name="Scan" component={Scan} />
        <Tab.Screen name="Search" component={SearchItems} />
        <Tab.Screen name="Settings" component={Settings} />
      </Tab.Navigator>
    </NavigationContainer>
  );
}



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