/*
 * LdapCommandLinePlugin.cpp - implementation of LdapCommandLinePlugin class
 *
 * Copyright (c) 2017 Tobias Doerffel <tobydox/at/users/dot/sf/dot/net>
 *
 * This file is part of iTALC - http://italc.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "Configuration/LocalStore.h"
#include "ItalcConfiguration.h"
#include "LdapCommandLinePlugin.h"
#include "Ldap/LdapDirectory.h"


LdapCommandLinePlugin::LdapCommandLinePlugin() :
	m_subCommands( {
				   std::pair<QString, QString>( "autoconfigurebasedn", "auto-configure the base DN via naming context" ),
				   std::pair<QString, QString>( "help", "show help about subcommand" ),
				   } )
{
}



LdapCommandLinePlugin::~LdapCommandLinePlugin()
{
}



QStringList LdapCommandLinePlugin::subCommands() const
{
	return m_subCommands.keys();
}



QString LdapCommandLinePlugin::subCommandHelp( const QString& subCommand ) const
{
	return m_subCommands.value( subCommand );
}



CommandLinePluginInterface::RunResult LdapCommandLinePlugin::runCommand( const QStringList& arguments )
{
	// all subcommands are handled as slots so if we land here an unsupported subcommand has been called
	return InvalidCommand;
}



CommandLinePluginInterface::RunResult LdapCommandLinePlugin::handle_autoconfigurebasedn( const QStringList& arguments )
{
	QUrl ldapUrl;
	ldapUrl.setUrl( arguments.value( 0 ), QUrl::StrictMode );

	if( ldapUrl.isValid() == false || ldapUrl.host().isEmpty() )
	{
		qCritical() << "Please specify a valid LDAP url following the schema \"ldap[s]://[user[:password]@]hostname[:port]\"";
		return InvalidArguments;
	}

	QString namingContextAttribute = arguments.value( 1 );

	if( namingContextAttribute.isEmpty() )
	{
		qWarning( "No naming context attribute name given - falling back to configured value." );
	}
	else
	{
		ItalcCore::config().setLdapNamingContextAttribute( namingContextAttribute );
	}

	LdapDirectory ldapDirectory( ldapUrl );
	QString baseDn = ldapDirectory.queryNamingContext();

	if( baseDn.isEmpty() )
	{
		qCritical( "Could not query base DN. Please check your LDAP configuration." );
		return Failed;
	}

	qInfo() << "Configuring" << baseDn << "as base DN and disabling naming context queries.";

	ItalcCore::config().setLdapBaseDn( baseDn );
	ItalcCore::config().setLdapQueryNamingContext( false );

	// write configuration
	Configuration::LocalStore localStore( Configuration::LocalStore::System );
	localStore.flush( &ItalcCore::config() );

	return Successful;
}



CommandLinePluginInterface::RunResult LdapCommandLinePlugin::handle_help( const QStringList& arguments )
{
	if( arguments.value( 0 ) == "autoconfigurebasedn" )
	{
		printf( "\n"
				"ldap autoconfigurebasedn <LDAP URL> [<naming context attribute name>]\n"
				"\n"
				"Automatically configures the LDAP base DN configuration setting by querying\n"
				"the naming context attribute from given LDAP server. The LDAP url parameter\n"
				"needs to follow the schema:\n"
				"\n"
				"  ldap[s]://[user[:password]@]hostname[:port]\n\n" );
		return NoResult;
	}

	return InvalidCommand;
}
