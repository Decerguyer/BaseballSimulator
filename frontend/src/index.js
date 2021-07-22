import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';

function Subtitle(props){
    return <h1> {props.text} </h1>;
}
function App(){
    return (
        <div>
            <Subtitle text="Welcome to the home page!" />
        </div>

    );
}

// ========================================

ReactDOM.render(
  <App />,
  document.getElementById('root')
);

