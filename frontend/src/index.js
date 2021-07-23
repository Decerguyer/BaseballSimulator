import React,{Component} from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import Table from "./components/Table";


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
                <Table pitches={this.state.pitches} />
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

