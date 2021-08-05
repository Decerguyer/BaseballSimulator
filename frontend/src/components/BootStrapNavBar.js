import React from 'react'
import {Navbar, Nav, NavDropdown, Form, FormControl, Button, Container} from 'react-bootstrap'

class BootstrapNavbar extends React.Component{

    render(){
        return(
            <Navbar bg="light" expand="lg">
                <Container>
                    <Navbar.Brand href="#home">Baseball Simulator</Navbar.Brand>
                    <Navbar.Toggle aria-controls="basic-navbar-nav" />
                    <Navbar.Collapse id="basic-navbar-nav">
                        <Nav className="me-auto">
                            <Nav.Link href="/">Home</Nav.Link>
                            <Nav.Link href="Dev.js">Dev</Nav.Link>
                            <NavDropdown title="More" id="basic-nav-dropdown">
                                <NavDropdown.Item href="#action/3.1">About us</NavDropdown.Item>
                                <NavDropdown.Item href="#action/3.2">Contact us</NavDropdown.Item>
                                <NavDropdown.Divider />
                                <NavDropdown.Item href="#action/3.4">Social Media</NavDropdown.Item>
                            </NavDropdown>
                        </Nav>
                    </Navbar.Collapse>
                </Container>
            </Navbar>
        )
    }
}

export default BootstrapNavbar;