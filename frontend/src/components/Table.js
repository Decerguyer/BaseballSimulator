import React, {Component} from 'react'
import './Table.css'

class Table extends Component{

    renderTableHeader(){
        let header = ['pitcher_id', 'pitch_id', 'serial_number', 'time']
        return header.map((key, index) => {
            return <th key={index}>{key.toUpperCase()}</th>
        })
    }
    renderTableData(){
        return this.props.pitches.map((val, key) =>{
            const { pitcher_id,error, pitch_id, positions, serial_number, spin,time,timestamps} = val
            return (
                <tr key={pitcher_id}>
                    <td>{pitcher_id}</td>
                    <td>{pitch_id}</td>
                    <td>{serial_number}</td>
                    <td>{time}</td>
                </tr>
            )

        })
    }

    render() {
        return (
            <div>
            <table id='pitches'>
                <tbody>
                    <tr>{this.renderTableHeader()}</tr>
                    {this.renderTableData()}
                </tbody>
            </table>
            </div>
        )
    }
}

export default Table