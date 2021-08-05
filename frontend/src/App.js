import React from "react";
import { BrowserRouter as Router, Route, Switch } from "react-router-dom";
import {Home, Navigation, Footer, Dev} from './components';

function App() {
    return (
        <div className="App">
            <Router>
                <Navigation />
                <Switch>
                    <Route path="/" exact component={() => <Home />} />
                    <Route path="/dev" exact component={() => <Dev />} />
                </Switch>
                <Footer />
            </Router>
        </div>
    );
}

export default App;