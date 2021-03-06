import React, { Component } from 'react';
import i18next from 'i18next';

import apiManager from '../lib/APIManager';
import messageDispatcher from '../lib/MessageDispatcher';

class GrantScope extends Component {
  constructor(props) {
    super(props);
    
    this.state = {
      config: props.config,
      currentUser: props.currentUser,
      client: props.client,
      scope: props.scope,
      show: props.show,
      infoSomeScopeUnavailable: props.infoSomeScopeUnavailable
    };
    
    this.handleToggleGrantScope = this.handleToggleGrantScope.bind(this);
    this.handleGrantScope = this.handleGrantScope.bind(this);
    
    messageDispatcher.subscribe('GrantScope', (message) => {
    });
	}
  
  componentWillReceiveProps(nextProps) {
    this.setState({
      config: nextProps.config, 
      currentUser: nextProps.currentUser, 
      client: nextProps.client, 
      scope: nextProps.scope,
      show: nextProps.show,
      infoSomeScopeUnavailable: nextProps.infoSomeScopeUnavailable
    });
  }

  handleToggleGrantScope(scope) {
    var scopeList = this.state.scope;
    scopeList.forEach((curScope) => {
      if (curScope.name === scope.name) {
        curScope.granted = !curScope.granted;
      }
    });
    this.setState({scope: scopeList});
  }

  handleGrantScope() {
    var scopeList = [];
    this.state.scope.forEach((scope) => {
      if (scope.granted) {
        scopeList.push(scope.name);
      }
    });
    apiManager.glewlwydRequest("/auth/grant/" + encodeURIComponent(this.state.client.client_id), "PUT", {scope: scopeList.join(" ")})
    .then(() => {
      messageDispatcher.sendMessage('App', {type: 'GrantComplete'});
    })
    .fail(() => {
      messageDispatcher.sendMessage('Notification', {type: "danger", message: i18next.t("login.error-set-grant")});
    });
  }
  
	render() {
    var scopeList = [];
    this.state.scope.forEach((scope, index) => {
      if (scope.name === "openid") {
        scopeList.push(
          <li className="list-group-item" key={index}>
            <div className="form-group form-check">
              <input type="checkbox" className="form-check-input" checked={true} disabled={true}/>
              <label className="form-check-label" htmlFor={"grant-" + scope.name}>{scope.display_name}</label>
            </div>
          </li>
        );
      } else {
        scopeList.push(
          <li className="list-group-item" key={index}>
            <div className="form-group form-check">
              <input type="checkbox" className="form-check-input" onChange={() => this.handleToggleGrantScope(scope)} id={"grant-" + scope.name} checked={scope.granted}/>
              <label className="form-check-label" htmlFor={"grant-" + scope.name}>{scope.display_name}</label>
            </div>
          </li>
        );
      }
    });
    var infoSomeScopeUnavailable;
    if (this.state.infoSomeScopeUnavailable) {
      infoSomeScopeUnavailable = <div className="alert alert-info" role="alert">{i18next.t("login.info-some-scope-unavailable")}</div>
    }
    return (
    <div>
      <div className="row">
        <div className="col-md-12">
          <h3>{i18next.t("login.grant-title")}</h3>
        </div>
      </div>
      <hr/>
      <div className="row">
        <div className="col-md-12">
          <h5>{i18next.t("login.grant-client-title", {client: this.state.client.name})}</h5>
        </div>
      </div>
      <div className="row">
        <div className="col-md-12">
          <ul className="list-group">
            {scopeList}
          </ul>
        </div>
      </div>
      <hr className="glwd-hr-no-border"/>
      <div className="row">
        <div className="col-md-2">
          <h5>{i18next.t("login.grant-client-id")}</h5>
        </div>
        <div className="col-md-10">
          <h5><span className="badge badge-secondary">{this.state.client.client_id}</span></h5>
        </div>
      </div>
      <div className="row">
        <div className="col-md-2">
          <h5>{i18next.t("login.grant-client-redirect-uri")}</h5>
        </div>
        <div className="col-md-10">
          <h5><span className="badge badge-secondary">{this.state.client.redirect_uri}</span></h5>
        </div>
      </div>
      <div className="row">
        <div className="col-md-12">
          {infoSomeScopeUnavailable}
        </div>
      </div>
      <hr/>
      <div className="row">
        <div className="col-md-12">
          <button type="button" className="btn btn-primary" onClick={this.handleGrantScope}>{i18next.t("login.grant")}</button>
        </div>
      </div>
      <hr className="glwd-hr-no-border"/>
      <div className="row">
        <div className="col-md-12">
          <b>
            {i18next.t("login.grant-info-message")}
          </b>
        </div>
      </div>
    </div>);
  }

}

export default GrantScope;
