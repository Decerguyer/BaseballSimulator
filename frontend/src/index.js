import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import Table from "./components/Table";
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
        return (
            <div>
                <BootstrapNavbar> </BootstrapNavbar>
                <div><Table pitches={this.state.pitches} /></div>

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

