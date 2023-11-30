// firebase.js

import { initializeApp } from 'firebase/app';
import { getDatabase, ref, get, set } from 'firebase/database';
import firebaseConfig from './firebaseConfig';

const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

const fetchChavesData = async () => {
  try {
    const snapshot = await get(ref(database));
    if (snapshot.exists()) {
      return snapshot.val();
    }
    return null;
  } catch (error) {
    console.error('Error fetching chaves:', error);
    throw error;
  }
};
const updateAbertura = async (value) => {
  try {
    await set(ref(database, 'abertura'), value);
  } catch (error) {
    console.error('Error updating abertura:', error);
    throw error;
  }
};

export { database, fetchChavesData, updateAbertura };