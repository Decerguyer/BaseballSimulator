import React,{Component} from 'react';
import ReactDOM from 'react-dom';
import './index.css';

function Subtitle(props){
    return <div><h1> {props.text.pitcher_id} </h1>
                <h1>{props.text.positions}</h1>
                <h1>{props.text.error}</h1>
                <h1>{props.text.serial_number}</h1>
                <h1>{props.text.spin}</h1>
                <h1>{props.text.time}</h1>
                <h1>{props.text.timestamps}</h1></div>;
}

class App extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            isLoaded: false,
            pitches: {
                "pitcher_id":"",
                "error": [],
                "pitch_id":"",
                "positions": [],
                "serial_number": "",
                "spin":[],
                "time":"",
                "timestamps":[],
                },
        };
    }

    render(){
        return (
            <div>
                <Subtitle text={this.state.pitches} />
            </div>
        )
    }


    componentDidMount() {
        fetch('/pitch')
            .then(res => res.json())
            .then(data=>{
                this.setState({pitches: data[0], isLoaded: true})
            })
            .catch(console.log);
    };


}


ReactDOM.render(
  <App />,
  document.getElementById('root')
);

