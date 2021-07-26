import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import Table from "./components/Table";
import Container from "./components/3dpitch";
import BootstrapNavbar from "./components/BootStrapNavBar";
import '../node_modules/bootstrap/dist/css/bootstrap.min.css';

class App extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            isLoaded: false,
            pitches: [{
                "pitcher_id":"",
                "error": [],
                "pitch_id":"",
                "positions": [[0,0,0], [0,0,0]],
                "serial_number": "",
                "spin":[],
                "time":"",
                "timestamps":[],
                }],
        };
    }

    render(){
        return (
            <div>
                <BootstrapNavbar> </BootstrapNavbar>
                <div><Table pitches={this.state.pitches} /></div>
                <Container trajectory={this.state.pitches[0].positions}/>

            </div>
        )
    }


    componentDidMount() {
        fetch('/pitch')
            .then(res => res.json())
            .then(data=>{
                this.setState({pitches: data, isLoaded: true})
            })
            .catch(console.log);
    };


}


ReactDOM.render(
  <App />,
  document.getElementById('root')
);

