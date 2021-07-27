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
                "positions": [],
                "serial_number": "",
                "spin":[],
                "time":"",
                "timestamps":[],
                }],
        };
    }

    render(){
        const {isLoaded, pitches} = this.state;
        return (
            isLoaded ?
                <div>
                    <BootstrapNavbar> </BootstrapNavbar>
                    {pitches.length > 0 && <><div><Table pitches={pitches} /></div>
                        <Container pitches={pitches}/></>}
                </div>:
                <div>
                    <BootstrapNavbar> </BootstrapNavbar>
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

