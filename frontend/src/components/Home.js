import React from 'react';
import Table from "./Table";
import Container from "./3dpitch";
import 'bootstrap/dist/css/bootstrap.min.css';
import 'semantic-ui-css/semantic.min.css';
import {Header} from "semantic-ui-react";

class Home extends React.Component{
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
                    <Header>Pitches Table</Header>
                    {pitches.length > 0 && <><div><Table pitches={pitches} /></div>
                        <Container pitches={pitches}/></>}
                </div>:
                <div/>
        )
    }


    componentDidMount() {
        fetch('https://pkafa2msue.execute-api.us-east-1.amazonaws.com/dev/pitch')
            .then(res => res.json())
            .then(data=>{
                this.setState({pitches: data, isLoaded: true})
            })
            .catch(console.log);
    };


}



export default Home;
