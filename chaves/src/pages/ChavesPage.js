// pages/ChavesPage.js

import { useEffect, useState } from 'react';
import { fetchChavesData,  updateAbertura } from '../firebase';
import '../app/global.css';

const ChavesPage = () => {
  const [chaves, setChaves] = useState([]);
  const [liberar, setLiberar] = useState([]);

  useEffect(() => {
    const fetchChaves = async () => {
      try {
        const chavesData = await fetchChavesData();
        if (chavesData) {
          const chavesArray = Object.values(chavesData);
          const chavesFiltradas = chavesArray
          .filter(chave => 
            chave.codChave !== undefined &&
            chave.codProf !== undefined &&
            chave.status !== undefined
          );
          setChaves(chavesFiltradas);
        }
      } catch (error) {
        console.error('Error fetching chaves:', error);
      }
    };

    fetchChaves();
  }, []);

  const enviaUm = () => {
    // Quando o botão for clicado, envia o valor 1
    setLiberar(1);
    updateAbertura(1);
  };

  const enviaZero = () => {
    // Quando o botão for clicado, envia o valor 0
    setLiberar(0);
    updateAbertura(0);
  };

  if (!chaves || chaves.length === 0) {
    return null;
  }

  return (
    <div className='container'>
      <h1>Chaves</h1>
      <ul>
        {chaves.map((chave, index) => (
          <li key={index} className='linha'>
            <strong>CodChave:</strong> {chave.codChave}<br />
            <strong>CodProf:</strong> {chave.codProf}<br />
            <strong>Status:</strong> {chave.status ? 'Ativo' : 'Inativo'}
          </li>
        ))}
      </ul>
      <button onClick={enviaUm}>Abrir</button>
      <button onClick={enviaZero}>Trancar</button>
    </div>
  );
};

export default ChavesPage;
